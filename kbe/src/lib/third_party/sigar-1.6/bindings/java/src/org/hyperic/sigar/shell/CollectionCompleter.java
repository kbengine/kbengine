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

import java.io.PrintStream;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.hyperic.sigar.util.GetlineCompleter;

/**
 * GetlineCompleter implementation looks for possible completions
 * using an Iterator.
 */
public class CollectionCompleter
    implements GetlineCompleter {

    private ArrayList completions = new ArrayList();
    private ShellBase shell = null;
    private PrintStream out = System.out;
    private Collection collection;

    public CollectionCompleter() { }

    public CollectionCompleter(ShellBase shell) {
        this.shell = shell;
        this.out = shell.getOutStream();
    }

    public CollectionCompleter(ShellBase shell, Collection collection) {
        this(shell);
        setCollection(collection);
    }

    public Iterator getIterator() {
        return getCollection().iterator();
    }

    public Collection getCollection() {
        return this.collection;
    }

    public void setCollection(Collection collection) {
        this.collection = collection;
    }

    private boolean startsWith(String substr, String[] possible) {
        for (int i=0; i<possible.length; i++) {
            if (!possible[i].startsWith(substr)) {
                return false;
            }
        }
        return true;
    }

    public String getPartialCompletion(String[] possible) {
        if (possible.length == 0) {
            return "";
        }
        String match = possible[0];

        StringBuffer lcd = new StringBuffer();

        for (int i=0; i<match.length(); i++) {
            if (startsWith(match.substring(0, i + 1), possible)) {
                lcd.append(match.charAt(i));
            }
            else {
                break;
            }
        }

        return lcd.toString();
    }

    public String displayPossible(List possible) {
        return displayPossible((String[])possible.
                               toArray(new String[possible.size()]));
    }

    public String displayPossible(String[] possible) {
        int size = possible.length;
        //print possibilities
        //XXX page possibilities

        String partial = getPartialCompletion(possible);

        for (int i=0; i<size; i++) {
            String match = possible[i];
            this.out.println();
            this.out.print(match + " ");
        }
        if (this.shell != null) {
            this.shell.getGetline().redraw();
        }
        if (partial.length() > 0) {
            return partial;
        }
        return null;
    }

    public String complete(String line) {
        this.completions.clear();
        int len = line.length();

        for (Iterator it = getIterator();
             it.hasNext(); )
        {
            String name = (String)it.next();
            if ((len == 0) || name.startsWith(line)) {
                this.completions.add(name);
            }
        }

        int size = this.completions.size();
        switch (size) {
          case 0:
            return line;
            
          case 1:
            return (String)this.completions.get(0);
            
          default:
            String partial = displayPossible(this.completions);
            if (partial != null) {
                return partial;
            }
            return line;
        }
    }
}
