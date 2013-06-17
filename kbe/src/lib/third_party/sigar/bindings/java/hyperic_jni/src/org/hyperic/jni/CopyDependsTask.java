/*
 * Copyright (c) 2009 Hyperic, Inc.
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

package org.hyperic.jni;

import java.io.File;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.taskdefs.Copy;

public class CopyDependsTask extends Copy {

    private File depends;

    protected void validateAttributes()
        throws BuildException {

        super.validateAttributes();
        if (this.depends == null) {
            throw new BuildException("missing depends attribute");
        }
    }

    public void execute()
        throws BuildException {

        if (this.destFile.exists()) {
            String state;
            if (this.depends.lastModified() >
                this.destFile.lastModified())
            {
                this.setOverwrite(true);
                state = "out of date";
            }
            else {
                state = "up to date";
            }

            log(this.destFile +
                " " + state + " with " +
                this.depends);
        }

        super.execute();
    }

    public File getDepends() {
        return this.depends;
    }

    public void setDepends(String depends) {
        this.depends = new File(depends);
    }
}
