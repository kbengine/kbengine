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

package org.hyperic.sigar.cmd;

import org.hyperic.sigar.DirUsage;
import org.hyperic.sigar.SigarException;

/**
 * Display usage for a directory recursively
 */
public class Du extends SigarCommandBase {

    //like du -s -b

    public Du(Shell shell) {
        super(shell);
    }

    public Du() {
        super();
    }

    public String getUsageShort() {
        return "Display usage for a directory recursively";
    }

    protected boolean validateArgs(String[] args) {
        return args.length == 1;
    }

    public void output(String[] args) throws SigarException {
        String dir = args[0];
        DirUsage du = this.sigar.getDirUsage(dir);
        println(du.getDiskUsage() + "\t" + dir);
    }

    public static void main(String[] args) throws Exception {
        new Du().processCommand(args);
    }
}
