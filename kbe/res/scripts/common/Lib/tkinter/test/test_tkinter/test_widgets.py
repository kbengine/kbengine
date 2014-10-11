import unittest
import tkinter
import os
import sys
from test.support import requires

from tkinter.test.support import (tcl_version, requires_tcl,
                                  get_tk_patchlevel, widget_eq)
from tkinter.test.widget_tests import (
    add_standard_options, noconv, pixels_round,
    AbstractWidgetTest, StandardOptionsTests, IntegerSizeTests, PixelSizeTests,
    setUpModule)

requires('gui')


def float_round(x):
    return float(round(x))


class AbstractToplevelTest(AbstractWidgetTest, PixelSizeTests):
    _conv_pad_pixels = noconv

    def test_class(self):
        widget = self.create()
        self.assertEqual(widget['class'],
                         widget.__class__.__name__.title())
        self.checkInvalidParam(widget, 'class', 'Foo',
                errmsg="can't modify -class option after widget is created")
        widget2 = self.create(class_='Foo')
        self.assertEqual(widget2['class'], 'Foo')

    def test_colormap(self):
        widget = self.create()
        self.assertEqual(widget['colormap'], '')
        self.checkInvalidParam(widget, 'colormap', 'new',
                errmsg="can't modify -colormap option after widget is created")
        widget2 = self.create(colormap='new')
        self.assertEqual(widget2['colormap'], 'new')

    def test_container(self):
        widget = self.create()
        self.assertEqual(widget['container'], 0 if self.wantobjects else '0')
        self.checkInvalidParam(widget, 'container', 1,
                errmsg="can't modify -container option after widget is created")
        widget2 = self.create(container=True)
        self.assertEqual(widget2['container'], 1 if self.wantobjects else '1')

    def test_visual(self):
        widget = self.create()
        self.assertEqual(widget['visual'], '')
        self.checkInvalidParam(widget, 'visual', 'default',
                errmsg="can't modify -visual option after widget is created")
        widget2 = self.create(visual='default')
        self.assertEqual(widget2['visual'], 'default')


@add_standard_options(StandardOptionsTests)
class ToplevelTest(AbstractToplevelTest, unittest.TestCase):
    OPTIONS = (
        'background', 'borderwidth',
        'class', 'colormap', 'container', 'cursor', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'menu', 'padx', 'pady', 'relief', 'screen',
        'takefocus', 'use', 'visual', 'width',
    )

    def _create(self, **kwargs):
        return tkinter.Toplevel(self.root, **kwargs)

    def test_menu(self):
        widget = self.create()
        menu = tkinter.Menu(self.root)
        self.checkParam(widget, 'menu', menu, eq=widget_eq)
        self.checkParam(widget, 'menu', '')

    def test_screen(self):
        widget = self.create()
        self.assertEqual(widget['screen'], '')
        try:
            display = os.environ['DISPLAY']
        except KeyError:
            self.skipTest('No $DISPLAY set.')
        self.checkInvalidParam(widget, 'screen', display,
                errmsg="can't modify -screen option after widget is created")
        widget2 = self.create(screen=display)
        self.assertEqual(widget2['screen'], display)

    def test_use(self):
        widget = self.create()
        self.assertEqual(widget['use'], '')
        parent = self.create(container=True)
        wid = parent.winfo_id()
        widget2 = self.create(use=wid)
        self.assertEqual(int(widget2['use']), wid)


@add_standard_options(StandardOptionsTests)
class FrameTest(AbstractToplevelTest, unittest.TestCase):
    OPTIONS = (
        'background', 'borderwidth',
        'class', 'colormap', 'container', 'cursor', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'relief', 'takefocus', 'visual', 'width',
    )

    def _create(self, **kwargs):
        return tkinter.Frame(self.root, **kwargs)


@add_standard_options(StandardOptionsTests)
class LabelFrameTest(AbstractToplevelTest, unittest.TestCase):
    OPTIONS = (
        'background', 'borderwidth',
        'class', 'colormap', 'container', 'cursor',
        'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'labelanchor', 'labelwidget', 'padx', 'pady', 'relief',
        'takefocus', 'text', 'visual', 'width',
    )

    def _create(self, **kwargs):
        return tkinter.LabelFrame(self.root, **kwargs)

    def test_labelanchor(self):
        widget = self.create()
        self.checkEnumParam(widget, 'labelanchor',
                            'e', 'en', 'es', 'n', 'ne', 'nw',
                            's', 'se', 'sw', 'w', 'wn', 'ws')
        self.checkInvalidParam(widget, 'labelanchor', 'center')

    def test_labelwidget(self):
        widget = self.create()
        label = tkinter.Label(self.root, text='Mupp', name='foo')
        self.checkParam(widget, 'labelwidget', label, expected='.foo')
        label.destroy()


class AbstractLabelTest(AbstractWidgetTest, IntegerSizeTests):
    _conv_pixels = noconv

    def test_highlightthickness(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'highlightthickness',
                              0, 1.3, 2.6, 6, -2, '10p')


@add_standard_options(StandardOptionsTests)
class LabelTest(AbstractLabelTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'activeforeground', 'anchor',
        'background', 'bitmap', 'borderwidth', 'compound', 'cursor',
        'disabledforeground', 'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'image', 'justify', 'padx', 'pady', 'relief', 'state',
        'takefocus', 'text', 'textvariable',
        'underline', 'width', 'wraplength',
    )

    def _create(self, **kwargs):
        return tkinter.Label(self.root, **kwargs)


@add_standard_options(StandardOptionsTests)
class ButtonTest(AbstractLabelTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'activeforeground', 'anchor',
        'background', 'bitmap', 'borderwidth',
        'command', 'compound', 'cursor', 'default',
        'disabledforeground', 'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'image', 'justify', 'overrelief', 'padx', 'pady', 'relief',
        'repeatdelay', 'repeatinterval',
        'state', 'takefocus', 'text', 'textvariable',
        'underline', 'width', 'wraplength')

    def _create(self, **kwargs):
        return tkinter.Button(self.root, **kwargs)

    def test_default(self):
        widget = self.create()
        self.checkEnumParam(widget, 'default', 'active', 'disabled', 'normal')


@add_standard_options(StandardOptionsTests)
class CheckbuttonTest(AbstractLabelTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'activeforeground', 'anchor',
        'background', 'bitmap', 'borderwidth',
        'command', 'compound', 'cursor',
        'disabledforeground', 'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'image', 'indicatoron', 'justify',
        'offrelief', 'offvalue', 'onvalue', 'overrelief',
        'padx', 'pady', 'relief', 'selectcolor', 'selectimage', 'state',
        'takefocus', 'text', 'textvariable',
        'tristateimage', 'tristatevalue',
        'underline', 'variable', 'width', 'wraplength',
    )

    def _create(self, **kwargs):
        return tkinter.Checkbutton(self.root, **kwargs)


    def test_offvalue(self):
        widget = self.create()
        self.checkParams(widget, 'offvalue', 1, 2.3, '', 'any string')

    def test_onvalue(self):
        widget = self.create()
        self.checkParams(widget, 'onvalue', 1, 2.3, '', 'any string')


@add_standard_options(StandardOptionsTests)
class RadiobuttonTest(AbstractLabelTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'activeforeground', 'anchor',
        'background', 'bitmap', 'borderwidth',
        'command', 'compound', 'cursor',
        'disabledforeground', 'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'image', 'indicatoron', 'justify', 'offrelief', 'overrelief',
        'padx', 'pady', 'relief', 'selectcolor', 'selectimage', 'state',
        'takefocus', 'text', 'textvariable',
        'tristateimage', 'tristatevalue',
        'underline', 'value', 'variable', 'width', 'wraplength',
    )

    def _create(self, **kwargs):
        return tkinter.Radiobutton(self.root, **kwargs)

    def test_value(self):
        widget = self.create()
        self.checkParams(widget, 'value', 1, 2.3, '', 'any string')


@add_standard_options(StandardOptionsTests)
class MenubuttonTest(AbstractLabelTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'activeforeground', 'anchor',
        'background', 'bitmap', 'borderwidth',
        'compound', 'cursor', 'direction',
        'disabledforeground', 'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'image', 'indicatoron', 'justify', 'menu',
        'padx', 'pady', 'relief', 'state',
        'takefocus', 'text', 'textvariable',
        'underline', 'width', 'wraplength',
    )
    _conv_pixels = staticmethod(pixels_round)

    def _create(self, **kwargs):
        return tkinter.Menubutton(self.root, **kwargs)

    def test_direction(self):
        widget = self.create()
        self.checkEnumParam(widget, 'direction',
                'above', 'below', 'flush', 'left', 'right')

    def test_height(self):
        widget = self.create()
        self.checkIntegerParam(widget, 'height', 100, -100, 0, conv=str)

    test_highlightthickness = StandardOptionsTests.test_highlightthickness

    @unittest.skipIf(sys.platform == 'darwin',
                     'crashes with Cocoa Tk (issue19733)')
    def test_image(self):
        widget = self.create()
        image = tkinter.PhotoImage('image1')
        self.checkParam(widget, 'image', image, conv=str)
        errmsg = 'image "spam" doesn\'t exist'
        with self.assertRaises(tkinter.TclError) as cm:
            widget['image'] = 'spam'
        if errmsg is not None:
            self.assertEqual(str(cm.exception), errmsg)
        with self.assertRaises(tkinter.TclError) as cm:
            widget.configure({'image': 'spam'})
        if errmsg is not None:
            self.assertEqual(str(cm.exception), errmsg)

    def test_menu(self):
        widget = self.create()
        menu = tkinter.Menu(widget, name='menu')
        self.checkParam(widget, 'menu', menu, eq=widget_eq)
        menu.destroy()

    def test_padx(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'padx', 3, 4.4, 5.6, '12m')
        self.checkParam(widget, 'padx', -2, expected=0)

    def test_pady(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'pady', 3, 4.4, 5.6, '12m')
        self.checkParam(widget, 'pady', -2, expected=0)

    def test_width(self):
        widget = self.create()
        self.checkIntegerParam(widget, 'width', 402, -402, 0, conv=str)


class OptionMenuTest(MenubuttonTest, unittest.TestCase):

    def _create(self, default='b', values=('a', 'b', 'c'), **kwargs):
        return tkinter.OptionMenu(self.root, None, default, *values, **kwargs)


@add_standard_options(IntegerSizeTests, StandardOptionsTests)
class EntryTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'background', 'borderwidth', 'cursor',
        'disabledbackground', 'disabledforeground',
        'exportselection', 'font', 'foreground',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'insertbackground', 'insertborderwidth',
        'insertofftime', 'insertontime', 'insertwidth',
        'invalidcommand', 'justify', 'readonlybackground', 'relief',
        'selectbackground', 'selectborderwidth', 'selectforeground',
        'show', 'state', 'takefocus', 'textvariable',
        'validate', 'validatecommand', 'width', 'xscrollcommand',
    )

    def _create(self, **kwargs):
        return tkinter.Entry(self.root, **kwargs)

    def test_disabledbackground(self):
        widget = self.create()
        self.checkColorParam(widget, 'disabledbackground')

    def test_insertborderwidth(self):
        widget = self.create(insertwidth=100)
        self.checkPixelsParam(widget, 'insertborderwidth',
                              0, 1.3, 2.6, 6, -2, '10p')
        # insertborderwidth is bounded above by a half of insertwidth.
        self.checkParam(widget, 'insertborderwidth', 60, expected=100//2)

    def test_insertwidth(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'insertwidth', 1.3, 3.6, '10p')
        self.checkParam(widget, 'insertwidth', 0.1, expected=2)
        self.checkParam(widget, 'insertwidth', -2, expected=2)
        if pixels_round(0.9) <= 0:
            self.checkParam(widget, 'insertwidth', 0.9, expected=2)
        else:
            self.checkParam(widget, 'insertwidth', 0.9, expected=1)

    def test_invalidcommand(self):
        widget = self.create()
        self.checkCommandParam(widget, 'invalidcommand')
        self.checkCommandParam(widget, 'invcmd')

    def test_readonlybackground(self):
        widget = self.create()
        self.checkColorParam(widget, 'readonlybackground')

    def test_show(self):
        widget = self.create()
        self.checkParam(widget, 'show', '*')
        self.checkParam(widget, 'show', '')
        self.checkParam(widget, 'show', ' ')

    def test_state(self):
        widget = self.create()
        self.checkEnumParam(widget, 'state',
                            'disabled', 'normal', 'readonly')

    def test_validate(self):
        widget = self.create()
        self.checkEnumParam(widget, 'validate',
                'all', 'key', 'focus', 'focusin', 'focusout', 'none')

    def test_validatecommand(self):
        widget = self.create()
        self.checkCommandParam(widget, 'validatecommand')
        self.checkCommandParam(widget, 'vcmd')


@add_standard_options(StandardOptionsTests)
class SpinboxTest(EntryTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'background', 'borderwidth',
        'buttonbackground', 'buttoncursor', 'buttondownrelief', 'buttonuprelief',
        'command', 'cursor', 'disabledbackground', 'disabledforeground',
        'exportselection', 'font', 'foreground', 'format', 'from',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'increment',
        'insertbackground', 'insertborderwidth',
        'insertofftime', 'insertontime', 'insertwidth',
        'invalidcommand', 'justify', 'relief', 'readonlybackground',
        'repeatdelay', 'repeatinterval',
        'selectbackground', 'selectborderwidth', 'selectforeground',
        'state', 'takefocus', 'textvariable', 'to',
        'validate', 'validatecommand', 'values',
        'width', 'wrap', 'xscrollcommand',
    )

    def _create(self, **kwargs):
        return tkinter.Spinbox(self.root, **kwargs)

    test_show = None

    def test_buttonbackground(self):
        widget = self.create()
        self.checkColorParam(widget, 'buttonbackground')

    def test_buttoncursor(self):
        widget = self.create()
        self.checkCursorParam(widget, 'buttoncursor')

    def test_buttondownrelief(self):
        widget = self.create()
        self.checkReliefParam(widget, 'buttondownrelief')

    def test_buttonuprelief(self):
        widget = self.create()
        self.checkReliefParam(widget, 'buttonuprelief')

    def test_format(self):
        widget = self.create()
        self.checkParam(widget, 'format', '%2f')
        self.checkParam(widget, 'format', '%2.2f')
        self.checkParam(widget, 'format', '%.2f')
        self.checkParam(widget, 'format', '%2.f')
        self.checkInvalidParam(widget, 'format', '%2e-1f')
        self.checkInvalidParam(widget, 'format', '2.2')
        self.checkInvalidParam(widget, 'format', '%2.-2f')
        self.checkParam(widget, 'format', '%-2.02f')
        self.checkParam(widget, 'format', '% 2.02f')
        self.checkParam(widget, 'format', '% -2.200f')
        self.checkParam(widget, 'format', '%09.200f')
        self.checkInvalidParam(widget, 'format', '%d')

    def test_from(self):
        widget = self.create()
        self.checkParam(widget, 'to', 100.0)
        self.checkFloatParam(widget, 'from', -10, 10.2, 11.7)
        self.checkInvalidParam(widget, 'from', 200,
                errmsg='-to value must be greater than -from value')

    def test_increment(self):
        widget = self.create()
        self.checkFloatParam(widget, 'increment', -1, 1, 10.2, 12.8, 0)

    def test_to(self):
        widget = self.create()
        self.checkParam(widget, 'from', -100.0)
        self.checkFloatParam(widget, 'to', -10, 10.2, 11.7)
        self.checkInvalidParam(widget, 'to', -200,
                errmsg='-to value must be greater than -from value')

    def test_values(self):
        # XXX
        widget = self.create()
        self.assertEqual(widget['values'], '')
        self.checkParam(widget, 'values', 'mon tue wed thur')
        self.checkParam(widget, 'values', ('mon', 'tue', 'wed', 'thur'),
                        expected='mon tue wed thur')
        self.checkParam(widget, 'values', (42, 3.14, '', 'any string'),
                        expected='42 3.14 {} {any string}')
        self.checkParam(widget, 'values', '')

    def test_wrap(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'wrap')

    def test_bbox(self):
        widget = self.create()
        bbox = widget.bbox(0)
        self.assertEqual(len(bbox), 4)
        for item in bbox:
            self.assertIsInstance(item, int)

        self.assertRaises(tkinter.TclError, widget.bbox, 'noindex')
        self.assertRaises(tkinter.TclError, widget.bbox, None)
        self.assertRaises(TypeError, widget.bbox)
        self.assertRaises(TypeError, widget.bbox, 0, 1)


@add_standard_options(StandardOptionsTests)
class TextTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'autoseparators', 'background', 'blockcursor', 'borderwidth',
        'cursor', 'endline', 'exportselection',
        'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'inactiveselectbackground', 'insertbackground', 'insertborderwidth',
        'insertofftime', 'insertontime', 'insertunfocussed', 'insertwidth',
        'maxundo', 'padx', 'pady', 'relief',
        'selectbackground', 'selectborderwidth', 'selectforeground',
        'setgrid', 'spacing1', 'spacing2', 'spacing3', 'startline', 'state',
        'tabs', 'tabstyle', 'takefocus', 'undo', 'width', 'wrap',
        'xscrollcommand', 'yscrollcommand',
    )
    if tcl_version < (8, 5):
        wantobjects = False

    def _create(self, **kwargs):
        return tkinter.Text(self.root, **kwargs)

    def test_autoseparators(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'autoseparators')

    @requires_tcl(8, 5)
    def test_blockcursor(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'blockcursor')

    @requires_tcl(8, 5)
    def test_endline(self):
        widget = self.create()
        text = '\n'.join('Line %d' for i in range(100))
        widget.insert('end', text)
        self.checkParam(widget, 'endline', 200, expected='')
        self.checkParam(widget, 'endline', -10, expected='')
        self.checkInvalidParam(widget, 'endline', 'spam',
                errmsg='expected integer but got "spam"')
        self.checkParam(widget, 'endline', 50)
        self.checkParam(widget, 'startline', 15)
        self.checkInvalidParam(widget, 'endline', 10,
                errmsg='-startline must be less than or equal to -endline')

    def test_height(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'height', 100, 101.2, 102.6, '3c')
        self.checkParam(widget, 'height', -100, expected=1)
        self.checkParam(widget, 'height', 0, expected=1)

    def test_maxundo(self):
        widget = self.create()
        self.checkIntegerParam(widget, 'maxundo', 0, 5, -1)

    @requires_tcl(8, 5)
    def test_inactiveselectbackground(self):
        widget = self.create()
        self.checkColorParam(widget, 'inactiveselectbackground')

    @requires_tcl(8, 6)
    def test_insertunfocussed(self):
        widget = self.create()
        self.checkEnumParam(widget, 'insertunfocussed',
                            'hollow', 'none', 'solid')

    def test_selectborderwidth(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'selectborderwidth',
                              1.3, 2.6, -2, '10p', conv=noconv,
                              keep_orig=tcl_version >= (8, 5))

    def test_spacing1(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'spacing1', 20, 21.4, 22.6, '0.5c')
        self.checkParam(widget, 'spacing1', -5, expected=0)

    def test_spacing2(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'spacing2', 5, 6.4, 7.6, '0.1c')
        self.checkParam(widget, 'spacing2', -1, expected=0)

    def test_spacing3(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'spacing3', 20, 21.4, 22.6, '0.5c')
        self.checkParam(widget, 'spacing3', -10, expected=0)

    @requires_tcl(8, 5)
    def test_startline(self):
        widget = self.create()
        text = '\n'.join('Line %d' for i in range(100))
        widget.insert('end', text)
        self.checkParam(widget, 'startline', 200, expected='')
        self.checkParam(widget, 'startline', -10, expected='')
        self.checkInvalidParam(widget, 'startline', 'spam',
                errmsg='expected integer but got "spam"')
        self.checkParam(widget, 'startline', 10)
        self.checkParam(widget, 'endline', 50)
        self.checkInvalidParam(widget, 'startline', 70,
                errmsg='-startline must be less than or equal to -endline')

    def test_state(self):
        widget = self.create()
        if tcl_version < (8, 5):
            self.checkParams(widget, 'state', 'disabled', 'normal')
        else:
            self.checkEnumParam(widget, 'state', 'disabled', 'normal')

    def test_tabs(self):
        widget = self.create()
        if get_tk_patchlevel() < (8, 5, 11):
            self.checkParam(widget, 'tabs', (10.2, 20.7, '1i', '2i'),
                            expected=('10.2', '20.7', '1i', '2i'))
        else:
            self.checkParam(widget, 'tabs', (10.2, 20.7, '1i', '2i'))
        self.checkParam(widget, 'tabs', '10.2 20.7 1i 2i',
                        expected=('10.2', '20.7', '1i', '2i'))
        self.checkParam(widget, 'tabs', '2c left 4c 6c center',
                        expected=('2c', 'left', '4c', '6c', 'center'))
        self.checkInvalidParam(widget, 'tabs', 'spam',
                               errmsg='bad screen distance "spam"',
                               keep_orig=tcl_version >= (8, 5))

    @requires_tcl(8, 5)
    def test_tabstyle(self):
        widget = self.create()
        self.checkEnumParam(widget, 'tabstyle', 'tabular', 'wordprocessor')

    def test_undo(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'undo')

    def test_width(self):
        widget = self.create()
        self.checkIntegerParam(widget, 'width', 402)
        self.checkParam(widget, 'width', -402, expected=1)
        self.checkParam(widget, 'width', 0, expected=1)

    def test_wrap(self):
        widget = self.create()
        if tcl_version < (8, 5):
            self.checkParams(widget, 'wrap', 'char', 'none', 'word')
        else:
            self.checkEnumParam(widget, 'wrap', 'char', 'none', 'word')

    def test_bbox(self):
        widget = self.create()
        bbox = widget.bbox('1.1')
        self.assertEqual(len(bbox), 4)
        for item in bbox:
            self.assertIsInstance(item, int)

        self.assertIsNone(widget.bbox('end'))
        self.assertRaises(tkinter.TclError, widget.bbox, 'noindex')
        self.assertRaises(tkinter.TclError, widget.bbox, None)
        self.assertRaises(TypeError, widget.bbox)
        self.assertRaises(TypeError, widget.bbox, '1.1', 'end')


@add_standard_options(PixelSizeTests, StandardOptionsTests)
class CanvasTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'background', 'borderwidth',
        'closeenough', 'confine', 'cursor', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'insertbackground', 'insertborderwidth',
        'insertofftime', 'insertontime', 'insertwidth',
        'relief', 'scrollregion',
        'selectbackground', 'selectborderwidth', 'selectforeground',
        'state', 'takefocus',
        'xscrollcommand', 'xscrollincrement',
        'yscrollcommand', 'yscrollincrement', 'width',
    )

    _conv_pixels = round
    wantobjects = False

    def _create(self, **kwargs):
        return tkinter.Canvas(self.root, **kwargs)

    def test_closeenough(self):
        widget = self.create()
        self.checkFloatParam(widget, 'closeenough', 24, 2.4, 3.6, -3,
                             conv=float)

    def test_confine(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'confine')

    def test_scrollregion(self):
        widget = self.create()
        self.checkParam(widget, 'scrollregion', '0 0 200 150')
        self.checkParam(widget, 'scrollregion', (0, 0, 200, 150),
                        expected='0 0 200 150')
        self.checkParam(widget, 'scrollregion', '')
        self.checkInvalidParam(widget, 'scrollregion', 'spam',
                               errmsg='bad scrollRegion "spam"')
        self.checkInvalidParam(widget, 'scrollregion', (0, 0, 200, 'spam'))
        self.checkInvalidParam(widget, 'scrollregion', (0, 0, 200))
        self.checkInvalidParam(widget, 'scrollregion', (0, 0, 200, 150, 0))

    def test_state(self):
        widget = self.create()
        self.checkEnumParam(widget, 'state', 'disabled', 'normal',
                errmsg='bad state value "{}": must be normal or disabled')

    def test_xscrollincrement(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'xscrollincrement',
                              40, 0, 41.2, 43.6, -40, '0.5i')

    def test_yscrollincrement(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'yscrollincrement',
                              10, 0, 11.2, 13.6, -10, '0.1i')


@add_standard_options(IntegerSizeTests, StandardOptionsTests)
class ListboxTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'activestyle', 'background', 'borderwidth', 'cursor',
        'disabledforeground', 'exportselection',
        'font', 'foreground', 'height',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'listvariable', 'relief',
        'selectbackground', 'selectborderwidth', 'selectforeground',
        'selectmode', 'setgrid', 'state',
        'takefocus', 'width', 'xscrollcommand', 'yscrollcommand',
    )

    def _create(self, **kwargs):
        return tkinter.Listbox(self.root, **kwargs)

    def test_activestyle(self):
        widget = self.create()
        self.checkEnumParam(widget, 'activestyle',
                            'dotbox', 'none', 'underline')

    def test_listvariable(self):
        widget = self.create()
        var = tkinter.DoubleVar()
        self.checkVariableParam(widget, 'listvariable', var)

    def test_selectmode(self):
        widget = self.create()
        self.checkParam(widget, 'selectmode', 'single')
        self.checkParam(widget, 'selectmode', 'browse')
        self.checkParam(widget, 'selectmode', 'multiple')
        self.checkParam(widget, 'selectmode', 'extended')

    def test_state(self):
        widget = self.create()
        self.checkEnumParam(widget, 'state', 'disabled', 'normal')

@add_standard_options(PixelSizeTests, StandardOptionsTests)
class ScaleTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'background', 'bigincrement', 'borderwidth',
        'command', 'cursor', 'digits', 'font', 'foreground', 'from',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'label', 'length', 'orient', 'relief',
        'repeatdelay', 'repeatinterval',
        'resolution', 'showvalue', 'sliderlength', 'sliderrelief', 'state',
        'takefocus', 'tickinterval', 'to', 'troughcolor', 'variable', 'width',
    )
    default_orient = 'vertical'

    def _create(self, **kwargs):
        return tkinter.Scale(self.root, **kwargs)

    def test_bigincrement(self):
        widget = self.create()
        self.checkFloatParam(widget, 'bigincrement', 12.4, 23.6, -5)

    def test_digits(self):
        widget = self.create()
        self.checkIntegerParam(widget, 'digits', 5, 0)

    def test_from(self):
        widget = self.create()
        self.checkFloatParam(widget, 'from', 100, 14.9, 15.1, conv=float_round)

    def test_label(self):
        widget = self.create()
        self.checkParam(widget, 'label', 'any string')
        self.checkParam(widget, 'label', '')

    def test_length(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'length', 130, 131.2, 135.6, '5i')

    def test_resolution(self):
        widget = self.create()
        self.checkFloatParam(widget, 'resolution', 4.2, 0, 6.7, -2)

    def test_showvalue(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'showvalue')

    def test_sliderlength(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'sliderlength',
                              10, 11.2, 15.6, -3, '3m')

    def test_sliderrelief(self):
        widget = self.create()
        self.checkReliefParam(widget, 'sliderrelief')

    def test_tickinterval(self):
        widget = self.create()
        self.checkFloatParam(widget, 'tickinterval', 1, 4.3, 7.6, 0,
                             conv=float_round)
        self.checkParam(widget, 'tickinterval', -2, expected=2,
                        conv=float_round)

    def test_to(self):
        widget = self.create()
        self.checkFloatParam(widget, 'to', 300, 14.9, 15.1, -10,
                             conv=float_round)


@add_standard_options(PixelSizeTests, StandardOptionsTests)
class ScrollbarTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'activerelief',
        'background', 'borderwidth',
        'command', 'cursor', 'elementborderwidth',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'jump', 'orient', 'relief',
        'repeatdelay', 'repeatinterval',
        'takefocus', 'troughcolor', 'width',
    )
    _conv_pixels = round
    wantobjects = False
    default_orient = 'vertical'

    def _create(self, **kwargs):
        return tkinter.Scrollbar(self.root, **kwargs)

    def test_activerelief(self):
        widget = self.create()
        self.checkReliefParam(widget, 'activerelief')

    def test_elementborderwidth(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'elementborderwidth', 4.3, 5.6, -2, '1m')

    def test_orient(self):
        widget = self.create()
        self.checkEnumParam(widget, 'orient', 'vertical', 'horizontal',
                errmsg='bad orientation "{}": must be vertical or horizontal')


@add_standard_options(StandardOptionsTests)
class PanedWindowTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'background', 'borderwidth', 'cursor',
        'handlepad', 'handlesize', 'height',
        'opaqueresize', 'orient', 'relief',
        'sashcursor', 'sashpad', 'sashrelief', 'sashwidth',
        'showhandle', 'width',
    )
    default_orient = 'horizontal'

    def _create(self, **kwargs):
        return tkinter.PanedWindow(self.root, **kwargs)

    def test_handlepad(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'handlepad', 5, 6.4, 7.6, -3, '1m')

    def test_handlesize(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'handlesize', 8, 9.4, 10.6, -3, '2m',
                              conv=noconv)

    def test_height(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'height', 100, 101.2, 102.6, -100, 0, '1i',
                              conv=noconv)

    def test_opaqueresize(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'opaqueresize')

    def test_sashcursor(self):
        widget = self.create()
        self.checkCursorParam(widget, 'sashcursor')

    def test_sashpad(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'sashpad', 8, 1.3, 2.6, -2, '2m')

    def test_sashrelief(self):
        widget = self.create()
        self.checkReliefParam(widget, 'sashrelief')

    def test_sashwidth(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'sashwidth', 10, 11.1, 15.6, -3, '1m',
                              conv=noconv)

    def test_showhandle(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'showhandle')

    def test_width(self):
        widget = self.create()
        self.checkPixelsParam(widget, 'width', 402, 403.4, 404.6, -402, 0, '5i',
                              conv=noconv)


@add_standard_options(StandardOptionsTests)
class MenuTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'activebackground', 'activeborderwidth', 'activeforeground',
        'background', 'borderwidth', 'cursor',
        'disabledforeground', 'font', 'foreground',
        'postcommand', 'relief', 'selectcolor', 'takefocus',
        'tearoff', 'tearoffcommand', 'title', 'type',
    )
    _conv_pixels = noconv

    def _create(self, **kwargs):
        return tkinter.Menu(self.root, **kwargs)

    def test_postcommand(self):
        widget = self.create()
        self.checkCommandParam(widget, 'postcommand')

    def test_tearoff(self):
        widget = self.create()
        self.checkBooleanParam(widget, 'tearoff')

    def test_tearoffcommand(self):
        widget = self.create()
        self.checkCommandParam(widget, 'tearoffcommand')

    def test_title(self):
        widget = self.create()
        self.checkParam(widget, 'title', 'any string')

    def test_type(self):
        widget = self.create()
        self.checkEnumParam(widget, 'type',
                'normal', 'tearoff', 'menubar')


@add_standard_options(PixelSizeTests, StandardOptionsTests)
class MessageTest(AbstractWidgetTest, unittest.TestCase):
    OPTIONS = (
        'anchor', 'aspect', 'background', 'borderwidth',
        'cursor', 'font', 'foreground',
        'highlightbackground', 'highlightcolor', 'highlightthickness',
        'justify', 'padx', 'pady', 'relief',
        'takefocus', 'text', 'textvariable', 'width',
    )
    _conv_pad_pixels = noconv

    def _create(self, **kwargs):
        return tkinter.Message(self.root, **kwargs)

    def test_aspect(self):
        widget = self.create()
        self.checkIntegerParam(widget, 'aspect', 250, 0, -300)


tests_gui = (
        ButtonTest, CanvasTest, CheckbuttonTest, EntryTest,
        FrameTest, LabelFrameTest,LabelTest, ListboxTest,
        MenubuttonTest, MenuTest, MessageTest, OptionMenuTest,
        PanedWindowTest, RadiobuttonTest, ScaleTest, ScrollbarTest,
        SpinboxTest, TextTest, ToplevelTest,
)

if __name__ == '__main__':
    unittest.main()
