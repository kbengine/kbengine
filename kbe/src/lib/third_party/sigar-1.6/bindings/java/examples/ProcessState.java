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

import org.hyperic.sigar.*;

/*

Example to show the process state for a given pid.

Compile the example:
% javac -classpath sigar-bin/lib/sigar.jar ProcessState.java

State of the java process running the example:
% java -classpath sigar-bin/lib/sigar.jar:. ProcessState
java: Running

State of the bash shell when invoking the example is running:
% java -classpath sigar-bin/lib/sigar.jar:. ProcessState $$
bash: Sleeping

State of emacs editor used to write the example:
% java -classpath sigar-bin/lib/sigar.jar:. ProcessState 2673
emacs: Suspended

See also: examples/Ps.java, examples/Top.java

*/

public class ProcessState {

    private static String getStateString(char state) {
        switch (state) {
          case ProcState.SLEEP:
            return "Sleeping";
          case ProcState.RUN:
            return "Running";
          case ProcState.STOP:
            return "Suspended";
          case ProcState.ZOMBIE:
            return "Zombie";
          case ProcState.IDLE:
            return "Idle";
          default:
            return String.valueOf(state);
        }
    }

    public static void main(String[] args)
        throws SigarException {

        String pid;
        if (args.length == 0) {
            pid = "$$"; //default to this process
        }
        else {
            pid = args[0];
        }

        Sigar sigar = new Sigar();

        ProcState procState = sigar.getProcState(pid);
        String state;

        System.out.println(procState.getName() + ": " +
                           getStateString(procState.getState()));

        sigar.close();
    }
}
