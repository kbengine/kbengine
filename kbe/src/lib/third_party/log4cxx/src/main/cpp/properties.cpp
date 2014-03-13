/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/helpers/inputstreamreader.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

class PropertyParser
{
public:
        void parse(LogString& in, Properties& properties)
        {
                LogString key, element;
                LexemType lexemType = BEGIN;
                logchar c;
                bool finished = false;

                if (!get(in, c))
                {
                        return;
                }

                while (!finished)
                {
                        switch(lexemType)
                        {
                        case BEGIN:
                                switch(c)
                                {
                                case 0x20: // ' '
                                case 0x08: // '\t'
                                case 0x0A: // '\n'
                                case 0x0D: // '\r'
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x23: // '#'
                                case 0x21: // '!'
                                        lexemType = COMMENT;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                default:
                                        lexemType = KEY;
                                        break;
                                }
                                break;

                        case KEY:
                                switch(c)
                                {
                                case 0x5C: // '\\'
                                        lexemType = KEY_ESCAPE;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x08: // '\t'
                                case 0x20: // ' '
                                case 0x3A: // ':'
                                case 0x3D: // '='
                                        lexemType = DELIMITER;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x0A:
                                case 0x0D:
                                        // key associated with an empty string element
                                        properties.setProperty(key, LogString());
                                        key.erase(key.begin(), key.end());
                                        lexemType = BEGIN;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                default:
                                        key.append(1, c);
                                        if (!get(in, c))
                                                finished = true;
                                        break;
                                }
                                break;

                        case KEY_ESCAPE:
                                switch(c)
                                {
                                case 0x08: // '\t'
                                case 0x20: // ' '
                                case 0x3A: // ':'
                                case 0x3D: // '='
                                case 0x5C: // '\\'
                                        key.append(1, c);
                                        lexemType = KEY;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x0A: // '\n'
                                        lexemType = KEY_CONTINUE;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x0D: // '\r'
                                        lexemType = KEY_CONTINUE2;
                                        if (!get(in, c))
                                                finished = true;
                                        break;
                                }
                                break;

                        case KEY_CONTINUE:
                                switch(c)
                                {
                                case 0x20:  // ' '
                                case 0x08: //  '\t'
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                default:
                                        lexemType = KEY;
                                        break;
                                }
                                break;

                        case KEY_CONTINUE2:
                                switch(c)
                                {
                                case 0x0A: // '\n'
                                        if (!get(in, c))
                                                finished = true;
                                        lexemType = KEY_CONTINUE;
                                        break;

                                default:
                                        lexemType = KEY_CONTINUE;
                                        break;
                                }
                                break;

                        case DELIMITER:
                                switch(c)
                                {
                                case 0x08: // '\t'
                                case 0x20: // ' '
                                case 0x3A: // ':'
                                case 0x3D: // '='
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                default:
                                        lexemType = ELEMENT;
                                        break;
                                }
                                break;

                        case ELEMENT:
                                switch(c)
                                {
                                case 0x5C: // '\\'
                                        lexemType = ELEMENT_ESCAPE;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x0A: // '\n'
                                case 0x0D: // '\r'
                                        // key associated with an empty string element
                                        properties.setProperty(key, element);
                                        key.erase(key.begin(), key.end());
                                        element.erase(element.begin(), element.end());
                                        lexemType = BEGIN;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                default:
                                        element.append(1, c);
                                        if (!get(in, c))
                                                finished = true;
                                        break;
                                }
                                break;

                        case ELEMENT_ESCAPE:
                                switch(c)
                                {
                                case 0x08: // '\t'
                                case 0x20: // ' '
                                case 0x6E: // 'n'
                                case 0x72: // 'r'
                                case 0x27: // '\''
                                case 0x5C: // '\\'
                                case 0x22: // '\"'
                                case 0x3A: // ':'
                                default:
                                        element.append(1, c);
                                        lexemType = ELEMENT;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x0A: // '\n'
                                        lexemType = ELEMENT_CONTINUE;
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                case 0x0D: // '\r'
                                        lexemType = ELEMENT_CONTINUE2;
                                        if (!get(in, c))
                                                finished = true;
                                        break;
                                }
                                break;

                        case ELEMENT_CONTINUE:
                                switch(c)
                                {
                                case 0x20: // ' '
                                case 0x08: // '\t'
                                        if (!get(in, c))
                                                finished = true;
                                        break;

                                default:
                                        lexemType = ELEMENT;
                                        break;
                                }
                                break;

                        case ELEMENT_CONTINUE2:
                                switch(c)
                                {
                                case 0x20: // '\n'
                                        if (!get(in, c))
                                                finished = true;
                                        lexemType = ELEMENT_CONTINUE;
                                        break;

                                default:
                                        lexemType = ELEMENT_CONTINUE;
                                        break;
                                }
                                break;

                        case COMMENT:
                                if (c == 0x0A || c == 0x0D)
                                {
                                        lexemType = BEGIN;
                                }
                                if (!get(in, c))
                                        finished = true;
                                break;
                        }
                }

                if (!key.empty())
                {
                        properties.setProperty(key, element);
                }
        }

protected:
        bool get(LogString& in, logchar& c)
        {
                if (in.empty()) {
                    c = 0;
                    return false;
                }
                c = in[0];
                in.erase(in.begin());
                return true;
        }

        typedef enum
        {
                BEGIN,
                KEY,
                KEY_ESCAPE,
                KEY_CONTINUE,
                KEY_CONTINUE2,
                DELIMITER,
                ELEMENT,
                ELEMENT_ESCAPE,
                ELEMENT_CONTINUE,
                ELEMENT_CONTINUE2,
                COMMENT
        }
        LexemType;
};

Properties::Properties() : properties(new PropertyMap()) {
}

Properties::~Properties() {
    delete properties;
}

LogString Properties::setProperty(const LogString& key, const LogString& value) {
    return put(key, value);
}

LogString Properties::put(const LogString& key, const LogString& value)
{
        LogString oldValue((*properties)[key]);
        (*properties)[key] = value;
        //tcout << ASCII_STR("setting property key=") << key << ASCII_STR(", value=") << value << std::endl;
        return oldValue;
}

LogString Properties::getProperty(const LogString& key) const {
    return get(key);
}

LogString Properties::get(const LogString& key) const
{
        PropertyMap::const_iterator it = properties->find(key);
        return (it != properties->end()) ? it->second : LogString();
}

void Properties::load(InputStreamPtr inStream) {
        Pool pool;
        InputStreamReaderPtr lineReader(
            new InputStreamReader(inStream, CharsetDecoder::getISOLatinDecoder()));
        LogString contents = lineReader->read(pool);
        properties->clear();
        PropertyParser parser;
        parser.parse(contents, *this);
}

std::vector<LogString> Properties::propertyNames() const
{
        std::vector<LogString> names;
        names.reserve(properties->size());

        PropertyMap::const_iterator it;
        for (it = properties->begin(); it != properties->end(); it++)
        {
                const LogString& key = it->first;
                names.push_back(key);
        }

        return names;
}

