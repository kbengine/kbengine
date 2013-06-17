/*
 * Copyright (c) 2008 Hyperic, Inc.
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

import java.io.File;
import java.io.IOException;
import java.util.Date;
import java.text.SimpleDateFormat;
import org.hyperic.sigar.FileInfo;
import org.hyperic.sigar.SigarException;

public class Ls extends SigarCommandBase {

    public Ls(Shell shell) {
        super(shell);
    }

    public Ls() {
        super();
    }

    public String getUsageShort() {
        return "simple FileInfo test at the moment (like ls -l)";
    }

    protected boolean validateArgs(String[] args) {
        return args.length == 1;
    }

    private String getDate(long mtime) {
        final String fmt = "MMM dd  yyyy";

        return new SimpleDateFormat(fmt).format(new Date(mtime));
    }

    public void output(String[] args) throws SigarException {
        String file = args[0];
        FileInfo link = this.sigar.getLinkInfo(file);
        FileInfo info = this.sigar.getFileInfo(file);
        if (link.getType() == FileInfo.TYPE_LNK) {
            try {
                file = file + " -> " + new File(file).getCanonicalPath();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        println(link.getTypeChar() + 
                info.getPermissionsString() + "\t" +
                info.getUid() + "\t" + info.getGid() + "\t" +
                info.getSize() + "\t" +
                getDate(info.getMtime()) + "\t" +
                file);
    }

    public static void main(String[] args) throws Exception {
        new Ls().processCommand(args);
    }
}
