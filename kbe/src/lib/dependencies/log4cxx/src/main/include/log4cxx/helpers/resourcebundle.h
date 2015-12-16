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

#ifndef _LOG4CXX_HELPERS_RESOURCE_BUNDLE_H
#define _LOG4CXX_HELPERS_RESOURCE_BUNDLE_H

#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
        namespace helpers
        {
                class Locale;

                class ResourceBundle;
                LOG4CXX_PTR_DEF(ResourceBundle);

                /**
                Resource bundles contain locale-specific objects
                */
                class LOG4CXX_EXPORT ResourceBundle : public ObjectImpl
                {
                public:
                        DECLARE_ABSTRACT_LOG4CXX_OBJECT(ResourceBundle)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(ResourceBundle)
                        END_LOG4CXX_CAST_MAP()

                        /**
                        Gets a string for the given key from this resource bundle or one of
                        its parents. Calling this method is equivalent to calling

                        @param key the key for the desired string
                        @return the string for the given key
                        @throw MissingResourceException - if no object for the given key
                        can be found
                        */
                        virtual LogString getString(const LogString& key) const = 0;

                        /**
                        Gets a resource bundle using the specified base name and locale

                        @param baseName the base name of the resource bundle, a fully
                        qualified class name or property filename
                        @param locale the locale for which a resource bundle is desired
                        */
                        static ResourceBundlePtr getBundle(const LogString& baseName,
                                const Locale& locale);

                protected:
                        /*
                        Sets the parent bundle of this bundle. The parent bundle is
                        searched by #getString when this bundle does not contain a particular
                        resource.

                        Parameters:
                        parent - this bundle's parent bundle.
                        */
                        inline void setParent(const ResourceBundlePtr& parent1)
                                { this->parent = parent1; }

                        /**
                        The parent bundle of this bundle.

                        The parent bundle is searched by #getString when this bundle does
                        not contain a particular resource.
                        */
                        ResourceBundlePtr parent;
                }; // class ResourceBundle
        }  // namespace helpers
} // namespace log4cxx

#endif

