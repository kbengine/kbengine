/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.hyperic.sigar.cmd;

import java.io.PrintStream;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarException;

import org.hyperic.sigar.pager.PageControl;
import org.hyperic.sigar.pager.PageFetchException;
import org.hyperic.sigar.pager.StaticPageFetcher;

import org.hyperic.sigar.util.GetlineCompleter;
import org.hyperic.sigar.util.PrintfFormat;

import org.hyperic.sigar.shell.CollectionCompleter;
import org.hyperic.sigar.shell.ProcessQueryCompleter;
import org.hyperic.sigar.shell.ShellCommandBase;
import org.hyperic.sigar.shell.ShellCommandExecException;
import org.hyperic.sigar.shell.ShellCommandUsageException;

public abstract class SigarCommandBase
    extends ShellCommandBase
    implements GetlineCompleter {

    protected Shell shell;
    protected PrintStream out = System.out;
    protected PrintStream err = System.err;
    protected Sigar sigar;
    protected SigarProxy proxy;
    protected List output = new ArrayList();
    private CollectionCompleter completer;
    private GetlineCompleter ptqlCompleter;
    private Collection completions = new ArrayList();
    private PrintfFormat formatter;
    private ArrayList printfItems = new ArrayList();

    public SigarCommandBase(Shell shell) {
        this.shell = shell;
        this.out   = shell.getOutStream();
        this.err   = shell.getErrStream();
        this.sigar = shell.getSigar();
        this.proxy = shell.getSigarProxy();
        
        //provide simple way for handlers to implement tab completion
        this.completer = new CollectionCompleter(shell);
        if (isPidCompleter()) {
            this.ptqlCompleter = new ProcessQueryCompleter(shell);
        }
    }

    public SigarCommandBase() {
        this(new Shell());
        this.shell.setPageSize(PageControl.SIZE_UNLIMITED);
    }

    public void setOutputFormat(String format) {
        this.formatter = new PrintfFormat(format);
    }

    public PrintfFormat getFormatter() {
        return this.formatter;
    }

    public String sprintf(String format, Object[] items) {
        return new PrintfFormat(format).sprintf(items);
    }

    public void printf(String format, Object[] items) {
        println(sprintf(format, items));
    }

    public void printf(Object[] items) {
        PrintfFormat formatter = getFormatter();
        if (formatter == null) {
            //see flushPrintfItems
            this.printfItems.add(items);
        }
        else {
            println(formatter.sprintf(items));
        }
    }

    public void printf(List items) {
        printf((Object[])items.toArray(new Object[0]));
    }

    public void println(String line) {
        if (this.shell.isInteractive()) {
            this.output.add(line);
        }
        else {
            this.out.println(line);
        }
    }

    private void flushPrintfItems() {
        if (this.printfItems.size() == 0) {
            return;
        }

        //no format was specified, just line up the columns
        int[] max = null;

        for (Iterator it=this.printfItems.iterator();
             it.hasNext();)
        {
            Object[] items = (Object[])it.next();
            if (max == null) {
                max = new int[items.length];
                Arrays.fill(max, 0);
            }
            for (int i=0; i<items.length; i++) {
                int len = items[i].toString().length();
                if (len > max[i]) {
                    max[i] = len;
                }
            }
        }

        StringBuffer format = new StringBuffer();
        for (int i=0; i<max.length; i++) {
            format.append("%-" + max[i] + "s");
            if (i < max.length-1) {
                format.append("    ");
            }
        }

        for (Iterator it=this.printfItems.iterator();
             it.hasNext();)
        {
            printf(format.toString(), (Object[])it.next());
        }
        this.printfItems.clear();
    }

    public void flush() {
        flushPrintfItems();

        try {
            this.shell.performPaging(new StaticPageFetcher(this.output));
        } catch(PageFetchException e) {
            this.err.println("Error paging: " + e.getMessage());
        } finally {
            this.output.clear();
        }
    }

    public abstract void output(String[] args)
        throws SigarException;

    protected boolean validateArgs(String[] args) {
        return args.length == 0;
    }

    public void processCommand(String[] args) 
        throws ShellCommandUsageException, ShellCommandExecException 
    {
        if (!validateArgs(args)) {
            throw new ShellCommandUsageException(getSyntax());
        }

        try {
            output(args);
        } catch (SigarException e) {
            throw new ShellCommandExecException(e.getMessage());
        }
    }

    public Collection getCompletions() {
        return this.completions;
    }

    public GetlineCompleter getCompleter() {
        return null;
    }

    public boolean isPidCompleter() {
        return false;
    }

    public String completePid(String line) {
        if ((line.length() >= 1) &&
            Character.isDigit(line.charAt(0)))
        {
            return line;
        }

        return this.ptqlCompleter.complete(line);
    }

    public String complete(String line) {
        if (isPidCompleter()) {
            return completePid(line);
        }
        GetlineCompleter c = getCompleter();
        if (c != null) {
            return c.complete(line);
        }

        this.completer.setCollection(getCompletions());
        return this.completer.complete(line);
    }
}
