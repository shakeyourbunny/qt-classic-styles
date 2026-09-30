# This file is part of the qt-classic-styles Project.
# License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
#
"""Tests for the Photon Aurorae theme generator.

The reference is the lossless QNX 6.2.1 file manager screenshot in
reference/qnx-photon/; every active frame piece must reproduce its crop
pixel for pixel.
"""
import configparser
import os
import stat
from pathlib import Path

import make_photon_aurorae as gen
import pytest

REPO = Path(__file__).resolve().parents[2]
REFERENCE = REPO / "reference" / "qnx-photon"

FRAME_PARTS = ("topleft", "top", "topright", "left", "center", "right",
               "bottomleft", "bottom", "bottomright")


@pytest.fixture(scope="module")
def theme(tmp_path_factory) -> Path:
    out = tmp_path_factory.mktemp("aurorae") / "Photon"
    gen.build_theme(REFERENCE, out)
    return out


def read_rc(theme: Path) -> configparser.ConfigParser:
    rc = configparser.ConfigParser(interpolation=None)
    rc.optionxform = str
    rc.read(theme / "Photonrc", encoding="utf-8")
    return rc


def test_build_writes_every_theme_file(theme):
    for name in ("decoration.svg", "minimize.svg", "maximize.svg", "restore.svg",
                 "close.svg", "Photonrc", "metadata.desktop"):
        assert (theme / name).is_file(), name


def test_decoration_has_active_inactive_and_maximized_elements(theme):
    elements = gen.read_elements((theme / "decoration.svg").read_text(encoding="utf-8"))
    for part in FRAME_PARTS:
        assert f"decoration-{part}" in elements
        assert f"decoration-inactive-{part}" in elements
    assert "decoration-maximized-center" in elements
    assert "decoration-maximized-inactive-center" in elements
    assert "hint-stretch-borders" in gen.element_ids(
        (theme / "decoration.svg").read_text(encoding="utf-8"))


CROPPED_FRAME = [p for p in gen.ACTIVE_FRAME if p.element != "decoration-bottomright"]


@pytest.mark.parametrize("piece", CROPPED_FRAME, ids=lambda p: p.element)
def test_active_frame_reproduces_reference_pixels(theme, piece):
    # bottomright is built from the bands instead (Photon's resize grip).
    elements = gen.read_elements((theme / "decoration.svg").read_text(encoding="utf-8"))
    expected = gen.repaired(gen.crop(REFERENCE / piece.source, piece.box))
    assert elements[piece.element] == expected


@pytest.mark.parametrize("piece", gen.BUTTON_CELLS, ids=lambda p: p.element)
def test_button_cells_reproduce_reference_pixels(theme, piece):
    elements = gen.read_elements((theme / f"{piece.element}.svg").read_text(encoding="utf-8"))
    assert elements["active-center"] == gen.repaired(gen.crop(REFERENCE / piece.source, piece.box))


def test_frame_sizes_agree_with_layout(theme):
    elements = gen.read_elements((theme / "decoration.svg").read_text(encoding="utf-8"))
    layout = read_rc(theme)["Layout"]
    top_height = len(elements["decoration-top"])
    assert top_height == (int(layout["TitleEdgeTop"]) + int(layout["TitleHeight"])
                          + int(layout["TitleEdgeBottom"]))
    assert len(elements["decoration-left"][0]) == int(layout["BorderLeft"])
    assert len(elements["decoration-right"][0]) == int(layout["BorderRight"])
    assert len(elements["decoration-bottom"]) == int(layout["BorderBottom"])
    assert len(elements["decoration-topleft"][0]) == int(layout["BorderLeft"])
    assert len(elements["decoration-topright"][0]) == int(layout["BorderRight"])


def test_button_sizes_agree_with_layout(theme):
    layout = read_rc(theme)["Layout"]
    widths = {"minimize": layout["ButtonWidthMinimize"],
              "maximize": layout["ButtonWidthMaximizeRestore"],
              "restore": layout["ButtonWidthMaximizeRestore"],
              "close": layout["ButtonWidthClose"]}
    for name, width in widths.items():
        grid = gen.read_elements((theme / f"{name}.svg").read_text(encoding="utf-8"))["active-center"]
        assert len(grid) == int(layout["ButtonHeight"]), name
        assert len(grid[0]) == int(width), name


def test_inactive_title_uses_inactive_colours(theme):
    elements = gen.read_elements((theme / "decoration.svg").read_text(encoding="utf-8"))
    top = elements["decoration-inactive-top"]
    # qnx621calc.png, the inactive calculator: light, mid, dark stripe, ramp.
    assert top[2][0] == (0xE3, 0xF3, 0xFF)
    assert top[3][0] == (0xB1, 0xC1, 0xD9)
    assert top[4][0] == (0x7F, 0x8F, 0xA7)
    assert top[6][0] == (0xB7, 0xC7, 0xDF)
    assert top[7][0] == (0xB4, 0xC4, 0xDC)
    for row in top:
        for r, g, b in row:
            assert not (b - r > 90), "active blue left in the inactive title"


def test_pressed_darkens_the_button_face_only(theme):
    elements = gen.read_elements((theme / "maximize.svg").read_text(encoding="utf-8"))
    active, pressed = elements["active-center"], elements["pressed-center"]
    ring = gen.button_ring(active)
    assert ring is not None
    left, top, right, bottom = ring
    for y in range(len(active)):
        for x in range(len(active[0])):
            inside = left < x < right and top < y < bottom
            if inside:
                assert sum(pressed[y][x]) < sum(active[y][x]), (x, y)
            else:
                assert pressed[y][x] == active[y][x], (x, y)


def test_deactivated_greys_out_the_glyph(theme):
    elements = gen.read_elements((theme / "close.svg").read_text(encoding="utf-8"))
    left, top, right, bottom = gen.button_ring(elements["active-center"])
    inner = [px for row in elements["deactivated-center"][top + 1:bottom]
             for px in row[left + 1:right]]
    assert min(max(px) for px in inner) >= 0x70


def test_rc_declares_centred_title_and_no_shadow(theme):
    general = read_rc(theme)["General"]
    assert general["TitleAlignment"] == "Center"
    assert general["Shadow"] == "false"
    assert general["ActiveTextColor"] == "0,0,101,255"


def test_crop_rejects_a_box_outside_the_image():
    with pytest.raises(ValueError):
        gen.crop(REFERENCE / "qnx621fileman.png", (790, 0, 900, 10))


def test_crop_rejects_an_empty_box():
    with pytest.raises(ValueError):
        gen.crop(REFERENCE / "qnx621fileman.png", (10, 10, 10, 20))


def test_build_refuses_a_missing_reference(tmp_path):
    with pytest.raises(FileNotFoundError):
        gen.build_theme(tmp_path / "nowhere", tmp_path / "out")


def _all_pixels(theme: Path):
    for svg in theme.glob("*.svg"):
        for element, grid in gen.read_elements(svg.read_text(encoding="utf-8")).items():
            for row in grid:
                for pixel in row:
                    yield svg.name, element, pixel


def test_generated_files_follow_the_umask(theme):
    umask = os.umask(0)
    os.umask(umask)
    for path in theme.iterdir():
        assert stat.S_IMODE(path.stat().st_mode) == 0o666 & ~umask, path.name


def test_swapped_channel_pixels_are_repaired(theme):
    # qnx621fileman.png has #AD592A where the title is #2A59AD: red and blue
    # swapped, a byte-order artifact of the original capture.
    for name, element, pixel in _all_pixels(theme):
        assert pixel != (0xAD, 0x59, 0x2A), (name, element)


def test_inactive_buttons_carry_no_active_blue(theme):
    for name in ("minimize", "maximize", "restore", "close"):
        elements = gen.read_elements((theme / f"{name}.svg").read_text(encoding="utf-8"))
        for state in ("inactive-center", "pressed-inactive-center", "deactivated-inactive-center"):
            for row in elements[state]:
                for r, _, b in row:
                    assert not (b - r > 90), (name, state)


def test_bottomright_corner_is_built_from_the_grey_bands(theme):
    elements = gen.read_elements((theme / "decoration.svg").read_text(encoding="utf-8"))
    corner = elements["decoration-bottomright"]
    right = elements["decoration-right"][0]
    bottom = [row[0] for row in elements["decoration-bottom"]]
    assert [row[4] for row in corner] == [right[4]] * 5
    assert corner[4] == [bottom[4]] * 5
    assert corner[0][0] == right[0]
    for row in corner:
        for r, _, b in row:
            assert not (b - r > 40)
