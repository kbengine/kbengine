/*
 * Copyright (c) 2006 Hyperic, Inc.
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

package org.hyperic.sigar.shell;

import java.io.BufferedReader;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.StringTokenizer;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.util.Getline;
import org.hyperic.sigar.util.GetlineCompleter;
import org.hyperic.sigar.util.IteratorIterator;

import org.hyperic.sigar.pager.PageControl;
import org.hyperic.sigar.pager.PageFetchException;
import org.hyperic.sigar.pager.PageFetcher;
import org.hyperic.sigar.pager.PageList;

public abstract class ShellBase
    implements ShellCommandMapper, GetlineCompleter, SIGINT {
    // Default size for pages when doing list commands
    public  static final String PROP_PAGE_SIZE    = "page.size";
    private static final int    DEFAULT_PAGE_SIZE = 20;

    private String name = null;
    private String prompt = null;
    private Map    handlers = null;
    private HashMap hiddenCommands;
    protected Getline gl;
    protected PrintStream out = System.out;
    protected PrintStream err = System.err;
    private boolean     doHistoryAdd;
    private int         pageSize;
    private boolean     isRedirected;
    private GetlineCompleter completer;

    public void handleSIGINT() {
        this.gl.reset();
    }

    public void initHistory() throws IOException {

        String historyFileName = 
            "." + this.name + "_history";

        initHistory(new File(System.getProperty("user.home"),
                             historyFileName));
    }

    public void initHistory(File file) throws IOException {

        this.doHistoryAdd = true;
        this.gl.initHistoryFile(file);
    }

    public void registerSigIntHandler() {
        ShellIntHandler.register(this); //catch ctrl-c
    }

    public void init (String applicationName, 
                      PrintStream out,
                      PrintStream err) {
        this.name         = applicationName;
        this.prompt       = applicationName;
        this.gl           = new Getline();
        this.out          = out;
        this.err          = err;
        this.doHistoryAdd = false;
        this.pageSize     = Integer.getInteger(PROP_PAGE_SIZE, 
                                               DEFAULT_PAGE_SIZE).intValue();
        if (this.pageSize != -1) {
            this.pageSize -= 1;
            if (this.pageSize < 1) {
                this.pageSize = 1;
            }
        }

        this.isRedirected = false;

        // Create command handler registry
        this.handlers = new HashMap();
        hiddenCommands = new HashMap();

        // Register help and quit commands
        try {
            ShellCommand_quit   quitCommand   = new ShellCommand_quit();
            ShellCommand_source sourceCommand = new ShellCommand_source();

            registerCommandHandler(".",      sourceCommand);
            registerCommandHandler("alias",  new ShellCommand_alias());
            registerCommandHandler("exit",   quitCommand);
            registerCommandHandler("get",    new ShellCommand_get());
            registerCommandHandler("help",   new ShellCommand_help());
            registerCommandHandler("q",      quitCommand);
            registerCommandHandler("quit",   quitCommand);
            registerCommandHandler("set",    new ShellCommand_set());
            registerCommandHandler("source", sourceCommand);
            registerCommandHandler("sleep",  new ShellCommand_sleep());
        } catch (Exception e) {
            err.println("ERROR: could not register standard commands: " + e);
            e.printStackTrace(err);
        }

        //DWIM commands
        setHandlerHidden(".", true);
        setHandlerHidden("q", true);
        setHandlerHidden("exit", true);

        registerSigIntHandler();

        this.completer = new CollectionCompleter(this) {
            public Iterator getIterator() {
                IteratorIterator it = new IteratorIterator();
                it.add(getCommandNameIterator());
                it.add(ShellCommand_alias.getAliases());
                return it;
            }
        };
    }

    /**
     * Read a .rc file into the shell, invoking everything in it (without
     * saving the actions to history)
     *
     * @param rcFile File to read
     */

    public void readRCFile(File rcFile, boolean echoCommands) 
        throws IOException 
    {
        FileInputStream is = null;
        boolean oldHistAdd = this.doHistoryAdd;

        this.doHistoryAdd = false;
        try {
            BufferedReader in;
            String line = null;

            is = new FileInputStream(rcFile);
            in = new BufferedReader(new InputStreamReader(is));
            while ((line = in.readLine()) != null) {
                line = line.trim();
                if (line.startsWith("#") || (line.length() == 0)) {
                    continue;
                }
                
                if (echoCommands) {
                    this.err.println(line);
                }

                handleCommand(line);
            }
        } finally {
            if (is != null) {
                is.close();
            }
            this.doHistoryAdd = oldHistAdd;
        }
    }

    /**
     * Change the prompt
     * @param prompt 
     */
    public void setPrompt(String prompt) {
        this.prompt = prompt;
    }

    /**
     * Register a new command handler.
     * @param commandName The command that this handler will process.
     * @param handler The handler to register.
     */
    public void registerCommandHandler(String commandName,
                                       ShellCommandHandler handler) 
        throws ShellCommandInitException {
        this.handlers.put(commandName, handler);
        handler.init(commandName, this);
    }

    /**
     * If a command needs additional input via the console, they 
     * can get it this way.
     * @param prompt The prompt to display.
     * @return The data that the user typed in.
     */
    public String getInput(String prompt) throws EOFException, IOException {
        return this.gl.getLine(prompt);
    }

    /**
     * If a command needs additional input via the console, they 
     * can get it this way.
     * @param prompt The prompt to display.
     * @param addToHistory If true, the input entered will be added to the 
     * history file.
     * @return The data that the user typed in.
     */
    public String getInput(String prompt, boolean addToHistory)
        throws EOFException, IOException {

        return this.gl.getLine(prompt, addToHistory);
    }

    /**
     * If a command needs additional input via the console, they 
     * can get it this way.  The characters that the user types
     * are not echoed.
     * @param prompt The prompt to display.
     * @return The data that the user typed in.
     */
    public String getHiddenInput(String prompt)
        throws EOFException, IOException 
    {
        return Sigar.getPassword(prompt);
    }

    /**
     * Write a string to this shell's output stream.
     * @param s The string to write to the output stream.
     */
    public void sendToOutStream(String s) {
        out.println(s);
    }

    /**
     * Write a string to this shell's output stream.
     * @param s The string to write to the output stream.
     */
    public void sendToErrStream(String s) {
        err.println(s);
    }

    public void run() {
        String input = null;

        ShellIntHandler.push(this);

        while (true) {
            try {
                // We don't add it to the history until we know
                // that it is not an illegal command
                input = this.gl.getLine(this.prompt + "> ", false);
            } catch (EOFException e) {
                break;
            } catch (Exception e) {
                err.println("Fatal error reading input line: " + e);
                e.printStackTrace(err);
                return;
            }
            if (input == null || input.trim().length() == 0) {
                if (!Getline.isTTY()) {
                    break;
                }
                else {
                    continue;
                }
            }

            try {
                handleCommand(input);
            } catch (NormalQuitCommandException nqce) {
                break;
            }
        }
        if (Getline.isTTY()) {
            out.println("Goodbye.");
        }
    }

    public void handleCommand(String line) {
        String[] args;

        try {
            args = explodeQuoted(line);
        } catch(IllegalArgumentException exc) {
            this.out.println("Syntax error: Unbalanced quotes");
            return;
        }

        if (args.length != 0) {
            handleCommand(line, args);
        }
    }

    public void handleCommand(String line, String[] args) {
        ShellCommandHandler handler = null;
        PrintStream oldSysOut = null, oldOut = null;
        String command = args[0];
        String[] subArgs;
        int useArgs;

        if (args.length == 0) {
            return;
        }

        handler = getHandler(command);
        if (handler == null) {
            String[] aliasArgs = ShellCommand_alias.getAlias(command);
            if (aliasArgs == null) {
                err.println("unknown command: " + command);
                return;
            }

            handleCommand(line, aliasArgs);
            return;
        }

        useArgs = args.length;
        if (args.length > 2 && args[args.length - 2].equals(">")) {
            PrintStream newOut;

            oldSysOut = System.out;
            oldOut    = this.out;

            // Re-direction, baby
            try {
                FileOutputStream fOut;

                fOut = new FileOutputStream(args[args.length -1]);
                newOut = new PrintStream(fOut);
            } catch(IOException exc) {
                this.err.println("Failed to redirect to output file: " + exc);
                return;
            }
            this.isRedirected = true;
            this.out = newOut;
            System.setOut(newOut);
            useArgs = useArgs - 2;
        } 

        subArgs = new String[useArgs - 1];
        System.arraycopy(args, 1, subArgs, 0, subArgs.length);
        
        try {
            processCommand(handler, subArgs);
        } catch (ShellCommandUsageException e) {
            String msg = e.getMessage();
            if (msg == null || msg.trim().length() == 0) {
                msg = "an unknown error occurred";
            }
            err.println(command + ": " + msg);

        } catch (ShellCommandExecException e) {
            err.println(e.getMessage());
        } catch (NormalQuitCommandException e) {
            throw e;
        } catch (Exception e) {
            err.println("Unexpected exception processing "
                        + "command '" + command + "': " + e);
            e.printStackTrace(err);

        } finally {
            if (this.doHistoryAdd) {
                this.gl.addToHistory(line);
            }

            if (oldSysOut != null) {
                this.isRedirected = false;
                System.setOut(oldSysOut);
                this.out = oldOut;
            }
        }
    }

    public void processCommand(ShellCommandHandler handler, String args[])
        throws ShellCommandUsageException, ShellCommandExecException
    {
        handler.processCommand(args);
    }

    public PrintStream getOutStream() {
        return this.out;
    }

    public PrintStream getErrStream() {
        return this.err;
    }

    public Getline getGetline() {
        return this.gl;
    }

    public boolean hasCompleter(ShellCommandHandler handler) {
        return GetlineCompleter.class.isAssignableFrom(handler.getClass());
    }

    public String complete(ShellCommandHandler handler, String line) {
        if (hasCompleter(handler)) {
            return ((GetlineCompleter)handler).complete(line);
        }

        return line;
    }

    public String complete(String line) {
        if (line == null) {
            return null;
        }
        int ix = line.indexOf(" ");

        if (ix != -1) {
            //if the command name has been completed
            //hand off completion of the rest to the command handler
            //if it implements GetlineHandler
            String cmd = line.substring(0, ix);
            String sub = line.substring(ix+1, line.length());
            ShellCommandHandler handler = getHandler(cmd);

            if (handler != null) {
                String hline = complete(handler, sub);
                return cmd + " " + hline;
            }

            return line;
        }

        line = this.completer.complete(line);

        if (getHandler(line) != null) {
            return line + " ";
        }

        return line;
    }

    /**
     * @see ShellCommandMapper#getHandler
     */
    public ShellCommandHandler getHandler(String command) {
        if (command == null) {
            return null;
        }

        return 
            (ShellCommandHandler)this.handlers.get(command.toLowerCase());
    }

    public void setHandlerHidden(String handlerName, boolean isHidden) {
        if (getHandler(handlerName) == null) {
            throw new IllegalArgumentException("Unknown handler: " +
                                               handlerName);
        }
        
        this.hiddenCommands.put(handlerName, 
                                isHidden ? Boolean.TRUE : Boolean.FALSE);
    }

    public boolean handlerIsHidden(String handlerName) {
        return this.hiddenCommands.get(handlerName) != null;
    }

    /**
     * @see ShellCommandMapper#getCommandNameIterator
     */
    public Iterator getCommandNameIterator() {
        ArrayList keyArray = new ArrayList();
        String[] keys;

        for (Iterator i = this.handlers.keySet().iterator();
            i.hasNext();)
        {
            String keyName = (String)i.next();
            
            if (!handlerIsHidden(keyName)) {
                keyArray.add(keyName);
            }
        }

        keys = (String[])keyArray.toArray(new String[0]);
        Arrays.sort(keys);
        return Arrays.asList(keys).iterator();
    }

    public void shutdown() {
    }

    /**
     * Check to see if the currently running shell command is being
     * redirected to a file.
     *
     * @return true if the shell is redirecting to a file, else false
     */
    public boolean isRedirected() {
        return this.isRedirected;
    }

    /**
     * Set the page size for data paging.  
     * 
     * @param size Number of rows to include in a page of data -- if
     *             0, then unlimited rows will be used.
     */
    public void setPageSize(int size) {
        if (size == 0 || size < -1) {
            throw new IllegalArgumentException("Page size must be > 0 or -1");
        }
        this.pageSize = size;
    }

    /**
     * Get the current page size used when paging data.
     *
     * @return the # of rows in the current page size.
     */
    public int getPageSize() {
        return this.pageSize;
    }

    /**
     * Get the number of pages that the fetcher can fetch, given the
     * settings as specified by the control and the # of total entites
     * the fetcher can fetch
     *
     * @param control    Control which dictates the page size
     * @param list       Last pageList queried via the control
     */
    private int getNumPages(PageControl control, PageList list) {
        int pageSize = control.getPagesize();
        int totalElems;

        totalElems = list.getTotalSize();

        if (pageSize == PageControl.SIZE_UNLIMITED) {
            return 1;
        }
        else if (pageSize == 0) {
            return 0;
        }

        if ((totalElems % pageSize) == 0) {
            return totalElems / pageSize;
        }

        return (totalElems / pageSize) + 1;
    }

    /**
     * Print a page of data
     * 
     * @param out              Stream to print to
     * @param data             List containing the data to print
     * @param lineNo           Line number of the first element of data
     * @param printLineNumbers If true, prefix lines with their numbers
     *
     * @return the number of lines printed
     */
    private void printPage(PrintStream out, PageList data, int lineNo,
                           boolean printLineNumbers)
    {
        for (Iterator i=data.iterator(); i.hasNext(); ) {
            if (printLineNumbers) {
                out.print(lineNo++ + ": ");
            }

            out.println((String)i.next());
        }
    }

    public PageControl getDefaultPageControl() {
        PageControl res;

        res = new PageControl(0, getPageSize() == -1 ? 
                              PageControl.SIZE_UNLIMITED :
                              getPageSize());
        return res;
    }

    public void performPaging(PageFetcher fetcher)
        throws PageFetchException
    {
        performPaging(fetcher, getDefaultPageControl());
    }

    public void performPaging(PageFetcher fetcher, PageControl control)
        throws PageFetchException
    {
        PrintStream out;
        PageList data;
        boolean lineNumberMode;

        // Don't know how to handle this case
        if (control.getPagesize() == 0) {
            return;
        }

        lineNumberMode = false;
        out            = getOutStream();

        if (isRedirected()) {
            control.setPagesize(PageControl.SIZE_UNLIMITED);
        }

        data = fetcher.getPage((PageControl)control.clone());
        printPage(out, data, control.getPageEntityIndex() + 1, 
                  lineNumberMode);

        if (control.getPagesize() == PageControl.SIZE_UNLIMITED ||
            data.size() < control.getPagesize())
        {
            return;
        }

        while (true) {
            boolean printPage = false;
            String cmd;
            int totalPages;

            totalPages = getNumPages(control, data);

            try {
                cmd = getInput("--More-- (Page " + 
                               (control.getPagenum() + 1) + " of " + 
                               totalPages + ")", false);
            } catch(IOException exc) {
                out.println();
                break;
            }

            if (cmd == null || (cmd = cmd.trim()).length() == 0) {
                printPage = true;
                control.setPagenum(control.getPagenum() + 1);
            }
            else if (cmd.equals("q")) {
                break;
            }
            else if (cmd.equals("b")) {
                printPage = true;
                if (control.getPagenum() > 0) {
                    control.setPagenum(control.getPagenum() - 1);
                }
            }
            else if (cmd.equals("l")) {
                lineNumberMode = !lineNumberMode;
                printPage = true;
            }
            else if (cmd.equals("?")) {
                out.println("  'b'        - Scroll back one page");
                out.println("  'l'        - Toggle line number mode");
                out.println("  'q'        - Quit paging");
                out.println("  '<number>' - Jump to the specified page #");
                out.println("  '<enter>'  - Scroll forward one page");
            }
            else {
                int newPageNo;

                try {
                    newPageNo = Integer.parseInt(cmd);
                } catch(NumberFormatException exc) {
                    out.println("Unknown command '" + cmd + "' " +
                                " type '?' for paging help");
                    continue;
                }
                
                if (newPageNo < 1 || newPageNo > totalPages) {
                    out.println(newPageNo + " out of range (must be " +
                                "1 to " + totalPages + ")");
                }
                else {
                    control.setPagenum(newPageNo - 1);
                    printPage = true;
                }
            }

            if (printPage) {
                data = fetcher.getPage((PageControl)control.clone());
                printPage(out, data, control.getPageEntityIndex() + 1,
                          lineNumberMode);

                // Check to see if we printed the last of the data
                if (data.size() < control.getPagesize()) {
                    break;
                }
            }
        }
    }

    private static String[] explodeQuoted(String arg) {
        ArrayList res = new ArrayList();
        StringTokenizer quoteTok;
        boolean inQuote = false;

        arg = arg.trim();
        quoteTok = new StringTokenizer(arg, "\"", true);

        while (quoteTok.hasMoreTokens()) {
            String elem = (String)quoteTok.nextElement();

            if (elem.equals("\"")) {
                inQuote = !inQuote;
                continue;
            }

            if (inQuote) {
                res.add(elem);
            }
            else {
                StringTokenizer spaceTok = new StringTokenizer(elem.trim());

                while (spaceTok.hasMoreTokens()) {
                    res.add(spaceTok.nextToken());
                }
            }
        }
        
        if (inQuote) {
            throw new IllegalArgumentException("Unbalanced quotation marks");
        }

        return (String[]) res.toArray(new String[0]);
    }
}
