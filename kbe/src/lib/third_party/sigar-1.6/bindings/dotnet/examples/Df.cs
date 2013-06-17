/*
 * Copyright (c) 2004-2005 Hyperic, Inc.
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

using System;
using Hyperic.Sigar;

public class Df {

    private static String FormatSize(long value) {
        return Sigar.FormatSize(value * 1024);
    }

    public static void Main() {
        Sigar sigar = new Sigar();

        foreach (FileSystem fs in sigar.FileSystemList()) {
            FileSystemUsage usage;
            long used, avail, total, pct;

            try {
                usage = sigar.FileSystemUsage(fs.DirName);

                used = usage.Total - usage.Free;
                avail = usage.Avail;
                total = usage.Total;
                pct = (long)(usage.UsePercent * 100);
            } catch (SigarException) {
                used = avail = total = pct = 0;
                continue;
            }

            string usePct;
            if (pct == 0) {
                usePct = "-";
            }
            else {
                usePct = pct + "%";
            }

            System.Console.WriteLine(fs.DevName + "\t" +
                                     FormatSize(total) + "\t" +
                                     FormatSize(used) + "\t" + 
                                     FormatSize(avail) + "\t" + 
                                     usePct + "\t" +
                                     fs.DirName + "\t" + 
                                     fs.SysTypeName + "/" + fs.TypeName);
        }
    }
}
