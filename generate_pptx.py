#!/usr/bin/env python3
"""
Generate PPTX from rotating-narrator JSON config.

Layout rules (derived from sample analysis):
- Slide: 19.05 cm × 33.87 cm (720×1280 portrait)
- Active phase lines: left=2.54cm, bottom of newest = 18.24cm, stacked upward
- Completed phases: rotated 90° or 270°, positioned off-slide to the side
- enter_rotation "left"  → previous phase rotated -90° (PPTX 270°), to the left
- enter_rotation "right" → previous phase rotated +90° (PPTX 90°),  to the right
- Rotations accumulate: each new phase transition adds ±90° to all prior phases
"""

import json
import re
import math
import os
import sys
from copy import deepcopy
from io import BytesIO

from pptx import Presentation
from pptx.util import Cm, Pt, Emu
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.oxml.ns import qn
from lxml import etree
from PIL import Image, ImageFilter

# ---------------------------------------------------------------------------
# Layout constants
# ---------------------------------------------------------------------------
SLIDE_W_CM = 19.05
SLIDE_H_CM = 33.87
LEFT_MARGIN_CM = 2.54          # left edge of all text
BOTTOM_ANCHOR_CM = 18.24       # bottom of the newest (largest) line
BASE_FONT_PT = 48              # font size for the newest line
SCALE_BACK = 0.82              # multiply per step backward (older = smaller)
LINE_GAP_CM = 0.35             # vertical gap between lines in same phase
TEXT_BOX_W_CM = 26.0           # wide enough to never wrap a single line



def cm(v): return Cm(v)
def pt(v): return Pt(v)


# ---------------------------------------------------------------------------
# Markup parser  {&font=X&color=#RRGGBB&size=+N}text{/}
# ---------------------------------------------------------------------------
MARKUP_RE = re.compile(r'\{&([^}]+)\}(.*?)\{/\}', re.DOTALL)

def parse_runs(text, default_font, default_color_hex, default_size_pt):
    """Return list of (segment_text, font_name, color_hex, size_pt)."""
    runs = []
    last = 0
    for m in MARKUP_RE.finditer(text):
        # plain text before this tag
        if m.start() > last:
            runs.append((text[last:m.start()], default_font,
                         default_color_hex, default_size_pt))
        attrs = {}
        for part in m.group(1).split('&'):
            if '=' in part:
                k, v = part.split('=', 1)
                attrs[k] = v
        font  = attrs.get('font',  default_font)
        color = attrs.get('color', default_color_hex)
        size_raw = attrs.get('size', None)
        if size_raw is not None:
            try:
                delta = int(size_raw.lstrip('+'))
                size  = default_size_pt + delta
            except ValueError:
                size  = default_size_pt
        else:
            size = default_size_pt
        runs.append((m.group(2), font, color, size))
        last = m.end()
    # remainder
    if last < len(text):
        runs.append((text[last:], default_font, default_color_hex, default_size_pt))
    return runs


def has_markup(text):
    return bool(MARKUP_RE.search(text))


# ---------------------------------------------------------------------------
# Background image (blurred)
# ---------------------------------------------------------------------------
def make_blurred_bg(image_path, blur_radius, target_w_px=720, target_h_px=1280):
    img = Image.open(image_path).convert('RGB')
    img = img.resize((target_w_px, target_h_px), Image.LANCZOS)
    img = img.filter(ImageFilter.GaussianBlur(radius=blur_radius))
    buf = BytesIO()
    img.save(buf, format='JPEG', quality=85)
    buf.seek(0)
    return buf


def add_background(slide, image_path, blur_radius):
    buf = make_blurred_bg(image_path, blur_radius)
    pic = slide.shapes.add_picture(buf, cm(0), cm(0), cm(SLIDE_W_CM), cm(SLIDE_H_CM))
    # Move picture to back (z-order)
    slide.shapes._spTree.remove(pic._element)
    slide.shapes._spTree.insert(2, pic._element)


# ---------------------------------------------------------------------------
# Text-box helpers
# ---------------------------------------------------------------------------
def line_height_cm(font_size_pt):
    """Approximate single-line text-box height for a given font size."""
    return font_size_pt / 72.0 * 2.54 * 1.45


def add_textbox(slide, text, default_font, default_color_hex, font_size_pt,
                left_cm, top_cm, width_cm, height_cm, rotation_deg=0):
    """Add a text box with optional inline markup and rotation."""
    from pptx.util import Cm as C
    txBox = slide.shapes.add_textbox(
        C(left_cm), C(top_cm), C(width_cm), C(height_cm))

    # Apply rotation via XML
    if rotation_deg != 0:
        sp = txBox._element
        spPr = sp.find(qn('p:spPr'))
        xfrm = spPr.find(qn('a:xfrm'))
        if xfrm is None:
            xfrm = etree.SubElement(spPr, qn('a:xfrm'))
        # PPTX rotation is in 1/60000 degree units, clockwise positive
        rot_val = int(rotation_deg * 60000)
        xfrm.set('rot', str(rot_val))

    tf = txBox.text_frame
    tf.word_wrap = False
    p = tf.paragraphs[0]

    runs = parse_runs(text, default_font, default_color_hex, font_size_pt)
    first = True
    for seg_text, font_name, color_hex, size_pt in runs:
        if not seg_text:
            continue
        if first:
            run = p.add_run()
            first = False
        else:
            run = p.add_run()
        run.text = seg_text
        run.font.name = font_name
        run.font.size = Pt(size_pt)
        try:
            r, g, b = int(color_hex[1:3], 16), int(color_hex[3:5], 16), int(color_hex[5:7], 16)
            run.font.color.rgb = RGBColor(r, g, b)
        except Exception:
            pass

    # Remove auto-fit so box doesn't resize
    txBody = tf._txBody
    bodyPr = txBody.find(qn('a:bodyPr'))
    if bodyPr is not None:
        bodyPr.attrib.pop('autofit', None)
        # Set no autofit
        for tag in [qn('a:noAutofit'), qn('a:spAutoFit'), qn('a:normAutofit')]:
            el = bodyPr.find(tag)
            if el is not None:
                bodyPr.remove(el)
        noaf = etree.SubElement(bodyPr, qn('a:noAutofit'))

    return txBox


# ---------------------------------------------------------------------------
# Core layout logic
# ---------------------------------------------------------------------------
def rotation_for_phase(phase_enter_rot):
    """Convert enter_rotation string to ±90 delta applied to PREVIOUS phases."""
    if phase_enter_rot == 'left':
        return -90    # previous phases rotate -90° (CCW), PPTX 270°
    elif phase_enter_rot == 'right':
        return +90    # previous phases rotate +90° (CW),  PPTX 90°
    return 0


def pptx_rot(deg):
    """Normalize degrees to [0,360) for PPTX."""
    return int(deg % 360)


def generate_slide(prs, slide_entries, current_phase_idx, last_trans_dir,
                   bg_image_path, blur_radius, default_font, default_color_hex):
    """
    slide_entries : list of entry dicts, oldest → newest.
    current_phase_idx : phase_idx of the currently active phase.
    last_trans_dir : 'left' | 'right' | 'none'
    """
    slide_layout = prs.slide_layouts[6]  # blank
    slide = prs.slides.add_slide(slide_layout)

    if bg_image_path and os.path.isfile(bg_image_path):
        add_background(slide, bg_image_path, blur_radius)

    # ── Active phase: horizontal lines, newest at BOTTOM_ANCHOR ──────────────
    current = [e for e in slide_entries if e['phase_idx'] == current_phase_idx]
    old     = [e for e in slide_entries if e['phase_idx'] != current_phase_idx]

    y_bottom = BOTTOM_ANCHOR_CM
    for entry in reversed(current):   # newest first
        h      = line_height_cm(entry['font_size_pt'])
        top_cm = y_bottom - h
        add_textbox(slide, entry['text'], default_font, default_color_hex,
                    entry['font_size_pt'],
                    LEFT_MARGIN_CM, top_cm, TEXT_BOX_W_CM, h)
        y_bottom = top_cm - LINE_GAP_CM

    # ── Old phases: packed outward from the slide edge ────────────────────────
    if not old:
        return slide

    phase_groups = {}
    for e in old:
        phase_groups.setdefault(e['phase_idx'], []).append(e)

    side = last_trans_dir   # 'left' or 'right'
    cy   = SLIDE_H_CM / 2.0  # vertical centre for all rotated columns

    # inner_limit: x-coordinate of the current packing boundary.
    # Left side: starts at 0 (slide left edge), moves more negative with each band.
    # Right side: starts at SLIDE_W_CM (slide right edge), moves more positive.
    inner_limit = 0.0 if side == 'left' else SLIDE_W_CM

    # Process phases nearest → furthest (highest phase_idx first)
    for pi in sorted(phase_groups.keys(), reverse=True):
        group    = phase_groups[pi]
        rot      = group[0]['phase_rotation']
        rot_norm = rot % 360

        if rot_norm in (90, 270):
            # ── Rotated ±90°: each line becomes a vertical column ──────────
            # Newest line is closest to the slide edge.
            # All columns are centred at cy = SLIDE_H/2.
            # Column visual thickness in the side direction = unrotated height h.
            for entry in reversed(group):   # newest first
                col_h    = line_height_cm(entry['font_size_pt'])
                rot_pptx = pptx_rot(rot)

                if side == 'left':
                    # Visual right edge of this column = inner_limit
                    center_x    = inner_limit - col_h / 2.0
                    inner_limit -= col_h
                else:
                    # Visual left edge of this column = inner_limit
                    center_x    = inner_limit + col_h / 2.0
                    inner_limit += col_h

                # In PPTX rotation is around the box centre, so:
                # box_left = center_x - w/2,  box_top = cy - col_h/2
                left_cm = center_x - TEXT_BOX_W_CM / 2.0
                top_cm  = cy - col_h / 2.0
                add_textbox(slide, entry['text'], default_font, default_color_hex,
                            entry['font_size_pt'],
                            left_cm, top_cm, TEXT_BOX_W_CM, col_h,
                            rotation_deg=rot_pptx)

        else:
            # ── Rotation 0° or 180°: horizontal, stacked like active phase ─
            # Vertical layout: same BOTTOM_ANCHOR, same gaps.
            y_b = BOTTOM_ANCHOR_CM
            for entry in reversed(group):   # newest first
                h      = line_height_cm(entry['font_size_pt'])
                top_cm = y_b - h
                if side == 'left':
                    left_cm      = inner_limit - TEXT_BOX_W_CM
                else:
                    left_cm      = inner_limit
                add_textbox(slide, entry['text'], default_font, default_color_hex,
                            entry['font_size_pt'],
                            left_cm, top_cm, TEXT_BOX_W_CM, h)
                y_b = top_cm - LINE_GAP_CM

            if side == 'left':
                inner_limit -= TEXT_BOX_W_CM
            else:
                inner_limit += TEXT_BOX_W_CM

    return slide


# ---------------------------------------------------------------------------
# Main entry point
# ---------------------------------------------------------------------------
def generate_pptx(json_path, output_path):
    base_dir = os.path.dirname(json_path)

    with open(json_path, encoding='utf-8') as f:
        config = json.load(f)

    defaults     = config.get('defaults', {})
    default_font = defaults.get('font', '黑体')
    default_color = defaults.get('color', '#000000')
    bg_file      = defaults.get('background', {}).get('file', '')
    bg_blur      = defaults.get('background', {}).get('blur', 0)
    bg_path      = os.path.join(base_dir, bg_file) if bg_file else ''

    phases = config['phases']

    prs = Presentation()
    prs.slide_width  = cm(SLIDE_W_CM)
    prs.slide_height = cm(SLIDE_H_CM)

    all_entries    = []
    last_trans_dir = 'none'   # tracks last transition direction for side placement

    for phase_idx, phase in enumerate(phases):
        enter_rot_str = phase.get('enter_rotation', 'none')
        rot_delta     = rotation_for_phase(enter_rot_str)

        # When this phase starts, rotate ALL previous entries
        if rot_delta != 0:
            for e in all_entries:
                e['phase_rotation'] = (e['phase_rotation'] + rot_delta)
            last_trans_dir = enter_rot_str   # 'left' or 'right'

        for line_idx, srt_entry in enumerate(phase['srt']):
            _start, _end, text, _anim = srt_entry

            entry = {
                'text':              text,
                'font_size_pt':      BASE_FONT_PT,
                'phase_idx':         phase_idx,
                'line_idx_in_phase': line_idx,
                'phase_rotation':    0,
            }
            all_entries.append(entry)

            # Recompute font sizes for all lines in the current phase
            cur = [e for e in all_entries if e['phase_idx'] == phase_idx]
            n   = len(cur)
            for k, e in enumerate(cur):
                steps_back = (n - 1) - k   # 0 = newest
                e['font_size_pt'] = BASE_FONT_PT * (SCALE_BACK ** steps_back)

            generate_slide(prs, all_entries, phase_idx, last_trans_dir,
                           bg_path, bg_blur, default_font, default_color)

    prs.save(output_path)
    print(f"Saved: {output_path}  ({len(prs.slides)} slides)")


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: generate_pptx.py <input.json> <output.pptx>")
        sys.exit(1)
    generate_pptx(sys.argv[1], sys.argv[2])
