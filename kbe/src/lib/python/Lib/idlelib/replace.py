"""Replace dialog for IDLE. Inherits SearchDialogBase for GUI.
Uses idlelib.SearchEngine for search capability.
Defines various replace related functions like replace, replace all,
replace+find.
"""
import re

from tkinter import StringVar, TclError

from idlelib.searchbase import SearchDialogBase
from idlelib import searchengine

def replace(text):
    """Returns a singleton ReplaceDialog instance.The single dialog
     saves user entries and preferences across instances."""
    root = text._root()
    engine = searchengine.get(root)
    if not hasattr(engine, "_replacedialog"):
        engine._replacedialog = ReplaceDialog(root, engine)
    dialog = engine._replacedialog
    dialog.open(text)


class ReplaceDialog(SearchDialogBase):

    title = "Replace Dialog"
    icon = "Replace"

    def __init__(self, root, engine):
        SearchDialogBase.__init__(self, root, engine)
        self.replvar = StringVar(root)

    def open(self, text):
        """Display the replace dialog"""
        SearchDialogBase.open(self, text)
        try:
            first = text.index("sel.first")
        except TclError:
            first = None
        try:
            last = text.index("sel.last")
        except TclError:
            last = None
        first = first or text.index("insert")
        last = last or first
        self.show_hit(first, last)
        self.ok = 1

    def create_entries(self):
        """Create label and text entry widgets"""
        SearchDialogBase.create_entries(self)
        self.replent = self.make_entry("Replace with:", self.replvar)[0]

    def create_command_buttons(self):
        SearchDialogBase.create_command_buttons(self)
        self.make_button("Find", self.find_it)
        self.make_button("Replace", self.replace_it)
        self.make_button("Replace+Find", self.default_command, 1)
        self.make_button("Replace All", self.replace_all)

    def find_it(self, event=None):
        self.do_find(0)

    def replace_it(self, event=None):
        if self.do_find(self.ok):
            self.do_replace()

    def default_command(self, event=None):
        "Replace and find next."
        if self.do_find(self.ok):
            if self.do_replace():  # Only find next match if replace succeeded.
                                   # A bad re can cause it to fail.
                self.do_find(0)

    def _replace_expand(self, m, repl):
        """ Helper function for expanding a regular expression
            in the replace field, if needed. """
        if self.engine.isre():
            try:
                new = m.expand(repl)
            except re.error:
                self.engine.report_error(repl, 'Invalid Replace Expression')
                new = None
        else:
            new = repl

        return new

    def replace_all(self, event=None):
        """Replace all instances of patvar with replvar in text"""
        prog = self.engine.getprog()
        if not prog:
            return
        repl = self.replvar.get()
        text = self.text
        res = self.engine.search_text(text, prog)
        if not res:
            self.bell()
            return
        text.tag_remove("sel", "1.0", "end")
        text.tag_remove("hit", "1.0", "end")
        line = res[0]
        col = res[1].start()
        if self.engine.iswrap():
            line = 1
            col = 0
        ok = 1
        first = last = None
        # XXX ought to replace circular instead of top-to-bottom when wrapping
        text.undo_block_start()
        while 1:
            res = self.engine.search_forward(text, prog, line, col, 0, ok)
            if not res:
                break
            line, m = res
            chars = text.get("%d.0" % line, "%d.0" % (line+1))
            orig = m.group()
            new = self._replace_expand(m, repl)
            if new is None:
                break
            i, j = m.span()
            first = "%d.%d" % (line, i)
            last = "%d.%d" % (line, j)
            if new == orig:
                text.mark_set("insert", last)
            else:
                text.mark_set("insert", first)
                if first != last:
                    text.delete(first, last)
                if new:
                    text.insert(first, new)
            col = i + len(new)
            ok = 0
        text.undo_block_stop()
        if first and last:
            self.show_hit(first, last)
        self.close()

    def do_find(self, ok=0):
        if not self.engine.getprog():
            return False
        text = self.text
        res = self.engine.search_text(text, None, ok)
        if not res:
            self.bell()
            return False
        line, m = res
        i, j = m.span()
        first = "%d.%d" % (line, i)
        last = "%d.%d" % (line, j)
        self.show_hit(first, last)
        self.ok = 1
        return True

    def do_replace(self):
        prog = self.engine.getprog()
        if not prog:
            return False
        text = self.text
        try:
            first = pos = text.index("sel.first")
            last = text.index("sel.last")
        except TclError:
            pos = None
        if not pos:
            first = last = pos = text.index("insert")
        line, col = searchengine.get_line_col(pos)
        chars = text.get("%d.0" % line, "%d.0" % (line+1))
        m = prog.match(chars, col)
        if not prog:
            return False
        new = self._replace_expand(m, self.replvar.get())
        if new is None:
            return False
        text.mark_set("insert", first)
        text.undo_block_start()
        if m.group():
            text.delete(first, last)
        if new:
            text.insert(first, new)
        text.undo_block_stop()
        self.show_hit(first, text.index("insert"))
        self.ok = 0
        return True

    def show_hit(self, first, last):
        """Highlight text from 'first' to 'last'.
        'first', 'last' - Text indices"""
        text = self.text
        text.mark_set("insert", first)
        text.tag_remove("sel", "1.0", "end")
        text.tag_add("sel", first, last)
        text.tag_remove("hit", "1.0", "end")
        if first == last:
            text.tag_add("hit", first)
        else:
            text.tag_add("hit", first, last)
        text.see("insert")
        text.update_idletasks()

    def close(self, event=None):
        SearchDialogBase.close(self, event)
        self.text.tag_remove("hit", "1.0", "end")


def _replace_dialog(parent):  # htest #
    from tkinter import Toplevel, Text, END, SEL
    from tkinter.ttk import Button

    box = Toplevel(parent)
    box.title("Test ReplaceDialog")
    x, y = map(int, parent.geometry().split('+')[1:])
    box.geometry("+%d+%d" % (x, y + 175))

    # mock undo delegator methods
    def undo_block_start():
        pass

    def undo_block_stop():
        pass

    text = Text(box, inactiveselectbackground='gray')
    text.undo_block_start = undo_block_start
    text.undo_block_stop = undo_block_stop
    text.pack()
    text.insert("insert","This is a sample sTring\nPlus MORE.")
    text.focus_set()

    def show_replace():
        text.tag_add(SEL, "1.0", END)
        replace(text)
        text.tag_remove(SEL, "1.0", END)

    button = Button(box, text="Replace", command=show_replace)
    button.pack()

if __name__ == '__main__':
    from unittest import main
    main('idlelib.idle_test.test_replace', verbosity=2, exit=False)

    from idlelib.idle_test.htest import run
    run(_replace_dialog)
