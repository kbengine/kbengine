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
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/appender.h>
#include <log4cxx/layout.h>
#include <log4cxx/logger.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/level.h>
#include <log4cxx/spi/filter.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/loader.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/config/propertysetter.h>
#include <log4cxx/spi/errorhandler.h>
#include <log4cxx/spi/loggerfactory.h>
#include <log4cxx/defaultloggerfactory.h>
#include <log4cxx/helpers/filewatchdog.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/spi/loggerrepository.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/pool.h>
#include <sstream>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/rolling/rollingfileappender.h>
#include <log4cxx/rolling/filterbasedtriggeringpolicy.h>
#include <apr_xml.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/charsetdecoder.h>
#include <log4cxx/net/smtpappender.h>

using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;
using namespace log4cxx::config;
using namespace log4cxx::rolling;


#if APR_HAS_THREADS
class XMLWatchdog  : public FileWatchdog
{
public:
        XMLWatchdog(const File& filename) : FileWatchdog(filename)
        {
        }

        /**
        Call DOMConfigurator#doConfigure with the
        <code>filename</code> to reconfigure log4cxx.
        */
        void doOnChange()
        {
                DOMConfigurator().doConfigure(file,
                        LogManager::getLoggerRepository());
        }
};
#endif


IMPLEMENT_LOG4CXX_OBJECT(DOMConfigurator)

#define CONFIGURATION_TAG "log4j:configuration"
#define OLD_CONFIGURATION_TAG "configuration"
#define APPENDER_TAG "appender"
#define APPENDER_REF_TAG "appender-ref"
#define PARAM_TAG "param"
#define LAYOUT_TAG "layout"
#define ROLLING_POLICY_TAG "rollingPolicy"
#define TRIGGERING_POLICY_TAG "triggeringPolicy"
#define CATEGORY "category"
#define LOGGER "logger"
#define LOGGER_REF "logger-ref"
#define CATEGORY_FACTORY_TAG "categoryFactory"
#define NAME_ATTR "name"
#define CLASS_ATTR "class"
#define VALUE_ATTR "value"
#define ROOT_TAG "root"
#define ROOT_REF "root-ref"
#define LEVEL_TAG "level"
#define PRIORITY_TAG "priority"
#define FILTER_TAG "filter"
#define ERROR_HANDLER_TAG "errorHandler"
#define REF_ATTR "ref"
#define ADDITIVITY_ATTR "additivity"
#define THRESHOLD_ATTR "threshold"
#define CONFIG_DEBUG_ATTR "configDebug"
#define INTERNAL_DEBUG_ATTR "debug"

DOMConfigurator::DOMConfigurator()
   : props(), repository() {
}

void DOMConfigurator::addRef() const {
   ObjectImpl::addRef();
}

void DOMConfigurator::releaseRef() const {
   ObjectImpl::releaseRef();
}

/**
Used internally to parse appenders by IDREF name.
*/
AppenderPtr DOMConfigurator::findAppenderByName(log4cxx::helpers::Pool& p,
                                                log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                                apr_xml_elem* element,
                                                apr_xml_doc* doc,
                                                const LogString& appenderName,
                                                AppenderMap& appenders) {
    AppenderPtr appender;
    std::string tagName(element->name);
    if (tagName == APPENDER_TAG) {
        if (appenderName == getAttribute(utf8Decoder, element, NAME_ATTR)) {
              appender = parseAppender(p, utf8Decoder, element, doc, appenders);
        }
    }
    if (element->first_child && !appender) {
         appender = findAppenderByName(p, utf8Decoder, element->first_child, doc, appenderName, appenders);
    }
    if (element->next && !appender) {
        appender = findAppenderByName(p, utf8Decoder, element->next, doc, appenderName, appenders);
    }
    return appender;
}

/**
 Used internally to parse appenders by IDREF element.
*/
AppenderPtr DOMConfigurator::findAppenderByReference(
                                                     log4cxx::helpers::Pool& p,
                                                     log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                                     apr_xml_elem* appenderRef,
                                                     apr_xml_doc* doc,
                                                     AppenderMap& appenders)
{
        LogString appenderName(subst(getAttribute(utf8Decoder, appenderRef, REF_ATTR)));
        AppenderMap::const_iterator match = appenders.find(appenderName);
        AppenderPtr appender;
        if (match != appenders.end()) {
            appender = match->second;
        } else if (doc) {
            appender = findAppenderByName(p, utf8Decoder, doc->root, doc, appenderName, appenders);
            if (appender) {
                appenders.insert(AppenderMap::value_type(appenderName, appender));
            }
        }
        if (!appender) {
                 LogLog::error(LOG4CXX_STR("No appender named [")+
                                appenderName+LOG4CXX_STR("] could be found."));
        }
        return appender;
}

/**
Used internally to parse an appender element.
*/
AppenderPtr DOMConfigurator::parseAppender(Pool& p,
                                           log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                           apr_xml_elem* appenderElement,
                                           apr_xml_doc* doc,
                                           AppenderMap& appenders)
{

    LogString className(subst(getAttribute(utf8Decoder, appenderElement, CLASS_ATTR)));
    LogLog::debug(LOG4CXX_STR("Class name: [") + className+LOG4CXX_STR("]"));
    try
        {
                ObjectPtr instance = Loader::loadClass(className).newInstance();
                AppenderPtr appender = instance;
                PropertySetter propSetter(appender);

                appender->setName(subst(getAttribute(utf8Decoder, appenderElement, NAME_ATTR)));

                for(apr_xml_elem* currentElement = appenderElement->first_child;
                     currentElement;
                     currentElement = currentElement->next) {

                                std::string tagName(currentElement->name);

                                // Parse appender parameters
                                if (tagName == PARAM_TAG)
                                {
                                        setParameter(p, utf8Decoder, currentElement, propSetter);
                                }
                                // Set appender layout
                                else if (tagName == LAYOUT_TAG)
                                {
                                        appender->setLayout(parseLayout(p, utf8Decoder, currentElement));
                                }
                                // Add filters
                                else if (tagName == FILTER_TAG)
                                {
                                        std::vector<log4cxx::spi::FilterPtr> filters;
                                        parseFilters(p, utf8Decoder, currentElement, filters);
                                        for(std::vector<log4cxx::spi::FilterPtr>::iterator iter = filters.begin();
                                            iter != filters.end();
                                            iter++) {
                                            appender->addFilter(*iter);
                                        }
                                }
                                else if (tagName == ERROR_HANDLER_TAG)
                                {
                                        parseErrorHandler(p, utf8Decoder, currentElement, appender, doc, appenders);
                                }
                                else if (tagName == ROLLING_POLICY_TAG)
                                {
                                        RollingPolicyPtr rollPolicy(parseRollingPolicy(p, utf8Decoder, currentElement));
                                        RollingFileAppenderPtr rfa(appender);
                                        if (rfa != NULL) {
                                           rfa->setRollingPolicy(rollPolicy);
                                        }
                                }
                                else if (tagName == TRIGGERING_POLICY_TAG)
                                {
                                        ObjectPtr policy(parseTriggeringPolicy(p, utf8Decoder, currentElement));
                                        RollingFileAppenderPtr rfa(appender);
                                        if (rfa != NULL) {
                                           rfa->setTriggeringPolicy(policy);
                                        } else {
                                            log4cxx::net::SMTPAppenderPtr smtpa(appender);
                                            if (smtpa != NULL) {
                                                log4cxx::spi::TriggeringEventEvaluatorPtr evaluator(policy);
                                                smtpa->setEvaluator(evaluator);
                                            }
                                        }
                                }
                                else if (tagName == APPENDER_REF_TAG)
                                {
                                        LogString refName = subst(getAttribute(utf8Decoder, currentElement, REF_ATTR));
                                        if(appender->instanceof(AppenderAttachable::getStaticClass()))
                                        {
                                                AppenderAttachablePtr aa(appender);
                                                LogLog::debug(LOG4CXX_STR("Attaching appender named [")+
                                                        refName+LOG4CXX_STR("] to appender named [")+
                                                        appender->getName()+LOG4CXX_STR("]."));
                                                aa->addAppender(findAppenderByReference(p, utf8Decoder, currentElement, doc, appenders));
                                        }
                                        else
                                        {
                                                LogLog::error(LOG4CXX_STR("Requesting attachment of appender named [")+
                                                        refName+ LOG4CXX_STR("] to appender named [")+ appender->getName()+
                                                        LOG4CXX_STR("] which does not implement AppenderAttachable."));
                                        }
                                }
                }
                propSetter.activate(p);
                return appender;
    }
    /* Yes, it's ugly.  But all of these exceptions point to the same
        problem: we can't create an Appender */
    catch (Exception& oops)
        {
                LogLog::error(LOG4CXX_STR("Could not create an Appender. Reported error follows."),
                        oops);
                return 0;
    }
}

/**
Used internally to parse an {@link ErrorHandler} element.
*/
void DOMConfigurator::parseErrorHandler(Pool& p,
                                        log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                        apr_xml_elem* element, 
                                        AppenderPtr& appender,
                                        apr_xml_doc* doc,
                                        AppenderMap& appenders)
{
    ErrorHandlerPtr eh = OptionConverter::instantiateByClassName(
                subst(getAttribute(utf8Decoder, element, CLASS_ATTR)),
                ErrorHandler::getStaticClass(),
                0);

    if(eh != 0)
        {
                eh->setAppender(appender);

                PropertySetter propSetter(eh);

                for (apr_xml_elem* currentElement = element->first_child;
                     currentElement;
                     currentElement = currentElement->next) {
                                std::string tagName(currentElement->name);
                                if(tagName == PARAM_TAG)
                                {
                                        setParameter(p, utf8Decoder, currentElement, propSetter);
                                }
                                else if(tagName == APPENDER_REF_TAG)
                                {
                                        eh->setBackupAppender(findAppenderByReference(p, utf8Decoder, currentElement, doc, appenders));
                                }
                                else if(tagName == LOGGER_REF)
                                {
                                        LogString loggerName(getAttribute(utf8Decoder, currentElement, REF_ATTR));
                                        LoggerPtr logger = repository->getLogger(loggerName, loggerFactory);
                                        eh->setLogger(logger);
                                }
                                else if(tagName == ROOT_REF)
                                {
                                        LoggerPtr root = repository->getRootLogger();
                                        eh->setLogger(root);
                                }
                }
                propSetter.activate(p);
//                appender->setErrorHandler(eh);
    }
}

/**
 Used internally to parse a filter element.
*/
void DOMConfigurator::parseFilters(Pool& p,                                 
                                   log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                   apr_xml_elem* element, 
                                   std::vector<log4cxx::spi::FilterPtr>& filters)
{
        LogString clazz = subst(getAttribute(utf8Decoder, element, CLASS_ATTR));
        FilterPtr filter = OptionConverter::instantiateByClassName(clazz,
                Filter::getStaticClass(), 0);

        if(filter != 0)
        {
                PropertySetter propSetter(filter);

                for (apr_xml_elem* currentElement = element->first_child;
                     currentElement;
                     currentElement = currentElement->next)
                {
                                std::string tagName(currentElement->name);
                                if(tagName == PARAM_TAG)
                                {
                                        setParameter(p, utf8Decoder, currentElement, propSetter);
                                }
                }
                propSetter.activate(p);
                filters.push_back(filter);
        }
}

/**
Used internally to parse an category or logger element.
*/
void DOMConfigurator::parseLogger(
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                  apr_xml_elem* loggerElement, 
                                  apr_xml_doc* doc,
                                  AppenderMap& appenders)
{
        // Create a new Logger object from the <category> element.
        LogString loggerName = subst(getAttribute(utf8Decoder, loggerElement, NAME_ATTR));

        LogLog::debug(LOG4CXX_STR("Retreiving an instance of Logger."));
        LoggerPtr logger = repository->getLogger(loggerName, loggerFactory);

        // Setting up a logger needs to be an atomic operation, in order
        // to protect potential log operations while logger
        // configuration is in progress.
        synchronized sync(logger->getMutex());
        bool additivity = OptionConverter::toBoolean(
                subst(getAttribute(utf8Decoder, loggerElement, ADDITIVITY_ATTR)),
                true);

        LogLog::debug(LOG4CXX_STR("Setting [")+logger->getName()+LOG4CXX_STR("] additivity to [")+
                (additivity ? LogString(LOG4CXX_STR("true")) : LogString(LOG4CXX_STR("false")))+LOG4CXX_STR("]."));
        logger->setAdditivity(additivity);
        parseChildrenOfLoggerElement(p, utf8Decoder, loggerElement, logger, false, doc, appenders);
}

/**
 Used internally to parse the logger factory element.
*/
void DOMConfigurator::parseLoggerFactory(
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                   apr_xml_elem* factoryElement)
{
        LogString className(subst(getAttribute(utf8Decoder, factoryElement, CLASS_ATTR)));

        if(className.empty())
        {
                LogLog::error(LOG4CXX_STR("Logger Factory tag class attribute not found."));
                LogLog::debug(LOG4CXX_STR("No Logger Factory configured."));
        }
        else
        {
                LogLog::debug(LOG4CXX_STR("Desired logger factory: [")+className+LOG4CXX_STR("]"));
                loggerFactory = OptionConverter::instantiateByClassName(
                        className,
                        LoggerFactory::getStaticClass(),
                        0);
                PropertySetter propSetter(loggerFactory);

                for (apr_xml_elem* currentElement = factoryElement->first_child;
                     currentElement;
                     currentElement = currentElement->next) {
                     std::string tagName(currentElement->name);
                     if (tagName == PARAM_TAG) {
                            setParameter(p, utf8Decoder, currentElement, propSetter);
                    }
                }
        }
}

/**
 Used internally to parse the root logger element.
*/
void DOMConfigurator::parseRoot(
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                  apr_xml_elem* rootElement, 
                                  apr_xml_doc* doc, 
                                  AppenderMap& appenders)
{
        LoggerPtr root = repository->getRootLogger();
        // logger configuration needs to be atomic
        synchronized sync(root->getMutex());
        parseChildrenOfLoggerElement(p, utf8Decoder, rootElement, root, true, doc, appenders);
}

/**
 Used internally to parse the children of a logger element.
*/
void DOMConfigurator::parseChildrenOfLoggerElement(
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                  apr_xml_elem* loggerElement, LoggerPtr logger, bool isRoot,
                                  apr_xml_doc* doc, 
                                  AppenderMap& appenders)
{

    PropertySetter propSetter(logger);

    // Remove all existing appenders from logger. They will be
    // reconstructed if need be.
    logger->removeAllAppenders();


    for (apr_xml_elem* currentElement = loggerElement->first_child;
         currentElement;
         currentElement = currentElement->next) {
                        std::string tagName(currentElement->name);

                        if (tagName == APPENDER_REF_TAG)
                        {
                                AppenderPtr appender = findAppenderByReference(p, utf8Decoder, currentElement, doc, appenders);
                                LogString refName =  subst(getAttribute(utf8Decoder, currentElement, REF_ATTR));
                                if(appender != 0)
                                {
                                        LogLog::debug(LOG4CXX_STR("Adding appender named [")+ refName+
                                        LOG4CXX_STR("] to logger [")+logger->getName()+LOG4CXX_STR("]."));
                                }
                                else
                                {
                                        LogLog::debug(LOG4CXX_STR("Appender named [")+ refName +
                                                LOG4CXX_STR("] not found."));
                                }

                                logger->addAppender(appender);

                        }
                        else if(tagName == LEVEL_TAG)
                        {
                                parseLevel(p, utf8Decoder, currentElement, logger, isRoot);
                        }
                        else if(tagName == PRIORITY_TAG)
                        {
                                parseLevel(p, utf8Decoder, currentElement, logger, isRoot);
                        }
                        else if(tagName == PARAM_TAG)
                        {
                                setParameter(p, utf8Decoder, currentElement, propSetter);
                        }
    }
    propSetter.activate(p);
}

/**
 Used internally to parse a layout element.
*/
LayoutPtr DOMConfigurator::parseLayout (
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                  apr_xml_elem* layout_element)
{
        LogString className(subst(getAttribute(utf8Decoder, layout_element, CLASS_ATTR)));
        LogLog::debug(LOG4CXX_STR("Parsing layout of class: \"")+className+LOG4CXX_STR("\""));
        try
        {
                ObjectPtr instance = Loader::loadClass(className).newInstance();
                LayoutPtr layout = instance;
                PropertySetter propSetter(layout);

                for(apr_xml_elem* currentElement = layout_element->first_child;
                    currentElement;
                    currentElement = currentElement->next) {
                                std::string tagName(currentElement->name);
                                if(tagName == PARAM_TAG)
                                {
                                        setParameter(p, utf8Decoder, currentElement, propSetter);
                                }
                }

                propSetter.activate(p);
                return layout;
        }
        catch (Exception& oops)
        {
                LogLog::error(LOG4CXX_STR("Could not create the Layout. Reported error follows."),
                        oops);
                return 0;
        }
}

/**
 Used internally to parse a triggering policy
*/
ObjectPtr DOMConfigurator::parseTriggeringPolicy (
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                  apr_xml_elem* layout_element)
{
        LogString className = subst(getAttribute(utf8Decoder, layout_element, CLASS_ATTR));
        LogLog::debug(LOG4CXX_STR("Parsing triggering policy of class: \"")+className+LOG4CXX_STR("\""));
        try
        {
                ObjectPtr instance = Loader::loadClass(className).newInstance();
                PropertySetter propSetter(instance);

                for (apr_xml_elem* currentElement = layout_element->first_child;
                     currentElement;
                     currentElement = currentElement->next) {
                                std::string tagName(currentElement->name);
                                if(tagName == PARAM_TAG)
                                {
                                        setParameter(p, utf8Decoder, currentElement, propSetter);
                                }
                                else if (tagName == FILTER_TAG) {
                                  std::vector<log4cxx::spi::FilterPtr> filters;
                                  parseFilters(p, utf8Decoder, currentElement, filters);
                                  FilterBasedTriggeringPolicyPtr fbtp(instance);
                                  if (fbtp != NULL) {
                                    for(std::vector<log4cxx::spi::FilterPtr>::iterator iter = filters.begin();
                                        iter != filters.end();
                                        iter++) {
                                        fbtp->addFilter(*iter);
                                    }
                                  }
                                }
                }

                propSetter.activate(p);
                return instance;
        }
        catch (Exception& oops)
        {
                LogLog::error(LOG4CXX_STR("Could not create the TriggeringPolicy. Reported error follows."),
                        oops);
                return 0;
        }
}

/**
 Used internally to parse a triggering policy
*/
RollingPolicyPtr DOMConfigurator::parseRollingPolicy (
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                  apr_xml_elem* layout_element)
{
        LogString className = subst(getAttribute(utf8Decoder, layout_element, CLASS_ATTR));
        LogLog::debug(LOG4CXX_STR("Parsing rolling policy of class: \"")+className+LOG4CXX_STR("\""));
        try
        {
                ObjectPtr instance = Loader::loadClass(className).newInstance();
                RollingPolicyPtr layout = instance;
                PropertySetter propSetter(layout);

                for(apr_xml_elem* currentElement = layout_element->first_child;
                    currentElement;
                    currentElement = currentElement->next) {
                                std::string tagName(currentElement->name);
                                if(tagName == PARAM_TAG)
                                {
                                        setParameter(p, utf8Decoder, currentElement, propSetter);
                        }
                }

                propSetter.activate(p);
                return layout;
        }
        catch (Exception& oops)
        {
                LogLog::error(LOG4CXX_STR("Could not create the RollingPolicy. Reported error follows."),
                        oops);
                return 0;
        }
}



/**
 Used internally to parse a level  element.
*/
void DOMConfigurator::parseLevel(
                                  log4cxx::helpers::Pool& p,
                                  log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,                                  
                                  apr_xml_elem* element, LoggerPtr logger, bool isRoot)
{
    LogString loggerName = logger->getName();
    if(isRoot)
        {
                loggerName = LOG4CXX_STR("root");
    }

    LogString levelStr(subst(getAttribute(utf8Decoder, element, VALUE_ATTR)));
        LogLog::debug(LOG4CXX_STR("Level value for ")+loggerName+LOG4CXX_STR(" is [")+levelStr+LOG4CXX_STR("]."));

    if (StringHelper::equalsIgnoreCase(levelStr,LOG4CXX_STR("INHERITED"), LOG4CXX_STR("inherited"))
        || StringHelper::equalsIgnoreCase(levelStr, LOG4CXX_STR("NULL"), LOG4CXX_STR("null")))
        {
                if(isRoot)
                {
                        LogLog::error(LOG4CXX_STR("Root level cannot be inherited. Ignoring directive."));
                }
                else
                {
                        logger->setLevel(0);
                }
    }
        else
        {
                LogString className(subst(getAttribute(utf8Decoder, element, CLASS_ATTR)));

                if (className.empty())
                {
                        logger->setLevel(OptionConverter::toLevel(levelStr, Level::getDebug()));
                }
                else
                {
                        LogLog::debug(LOG4CXX_STR("Desired Level sub-class: [") + className + LOG4CXX_STR("]"));

                        try
                        {
                                Level::LevelClass& levelClass =
                                        (Level::LevelClass&)Loader::loadClass(className);
                                LevelPtr level = levelClass.toLevel(levelStr);
                                logger->setLevel(level);
                        }
                        catch (Exception& oops)
                        {
                                LogLog::error(
                                        LOG4CXX_STR("Could not create level [") + levelStr +
                                        LOG4CXX_STR("]. Reported error follows."),
                                        oops);

                                return;
                        }
                        catch (...)
                        {
                                LogLog::error(
                                        LOG4CXX_STR("Could not create level [") + levelStr);

                                return;
                        }
                }
    }

        LogLog::debug(loggerName + LOG4CXX_STR(" level set to ") +
                logger->getEffectiveLevel()->toString());
}

void DOMConfigurator::setParameter(log4cxx::helpers::Pool& p,
                                log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                apr_xml_elem* elem, 
                                PropertySetter& propSetter)
{
        LogString name(subst(getAttribute(utf8Decoder, elem, NAME_ATTR)));
        LogString value(subst(getAttribute(utf8Decoder, elem, VALUE_ATTR)));
        value = subst(value);
        propSetter.setProperty(name, value, p);
}

void DOMConfigurator::doConfigure(const File& filename, spi::LoggerRepositoryPtr& repository1)
{
       repository1->setConfigured(true);
        this->repository = repository1;
        LogString msg(LOG4CXX_STR("DOMConfigurator configuring file "));
        msg.append(filename.getPath());
        msg.append(LOG4CXX_STR("..."));
        LogLog::debug(msg);

        loggerFactory = new DefaultLoggerFactory();

        Pool p;
        apr_file_t *fd;

        log4cxx_status_t rv = filename.open(&fd, APR_READ, APR_OS_DEFAULT, p);
        if (rv != APR_SUCCESS) {
            LogString msg2(LOG4CXX_STR("Could not open file ["));
            msg2.append(filename.getPath());
            msg2.append(LOG4CXX_STR("]."));
            LogLog::error(msg2);
        } else {
            apr_xml_parser *parser;
            apr_xml_doc *doc;
            rv = apr_xml_parse_file(p.getAPRPool(), &parser, &doc, fd, 2000);
            if (rv != APR_SUCCESS) {
                char errbuf[2000];
                char errbufXML[2000];
                LogString msg2(LOG4CXX_STR("Error parsing file ["));
                msg2.append(filename.getPath());
                msg2.append(LOG4CXX_STR("], "));
                apr_strerror(rv, errbuf, sizeof(errbuf));
                LOG4CXX_DECODE_CHAR(lerrbuf, std::string(errbuf));
                apr_xml_parser_geterror(parser, errbufXML, sizeof(errbufXML));
                LOG4CXX_DECODE_CHAR(lerrbufXML, std::string(errbufXML));
                msg2.append(lerrbuf);
                msg2.append(lerrbufXML);
                LogLog::error(msg2);
            } else {
                AppenderMap appenders;
                CharsetDecoderPtr utf8Decoder(CharsetDecoder::getUTF8Decoder());
                parse(p, utf8Decoder, doc->root, doc, appenders);
            }
        }
}

void DOMConfigurator::configure(const std::string& filename)
{
    File file(filename);
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
}

#if LOG4CXX_WCHAR_T_API
void DOMConfigurator::configure(const std::wstring& filename)
{
    File file(filename);
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
}
#endif

#if LOG4CXX_UNICHAR_API
void DOMConfigurator::configure(const std::basic_string<UniChar>& filename)
{
    File file(filename);
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
}
#endif

#if LOG4CXX_CFSTRING_API
void DOMConfigurator::configure(const CFStringRef& filename)
{
    File file(filename);
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
}
#endif


void DOMConfigurator::configureAndWatch(const std::string& filename)
{
  configureAndWatch(filename, FileWatchdog::DEFAULT_DELAY);
}

#if LOG4CXX_WCHAR_T_API
void DOMConfigurator::configureAndWatch(const std::wstring& filename)
{
  configureAndWatch(filename, FileWatchdog::DEFAULT_DELAY);
}
#endif

#if LOG4CXX_UNICHAR_API
void DOMConfigurator::configureAndWatch(const std::basic_string<UniChar>& filename)
{
  configureAndWatch(filename, FileWatchdog::DEFAULT_DELAY);
}
#endif

#if LOG4CXX_CFSTRING_API
void DOMConfigurator::configureAndWatch(const CFStringRef& filename)
{
  configureAndWatch(filename, FileWatchdog::DEFAULT_DELAY);
}
#endif

void DOMConfigurator::configureAndWatch(const std::string& filename, long delay)
{
        File file(filename);
#if APR_HAS_THREADS
        XMLWatchdog * xdog = new XMLWatchdog(file);
        xdog->setDelay(delay);
        xdog->start();
#else
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
#endif        
}

#if LOG4CXX_WCHAR_T_API
void DOMConfigurator::configureAndWatch(const std::wstring& filename, long delay)
{
        File file(filename);
#if APR_HAS_THREADS
        XMLWatchdog * xdog = new XMLWatchdog(file);
        xdog->setDelay(delay);
        xdog->start();
#else
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
#endif        
}
#endif

#if LOG4CXX_UNICHAR_API
void DOMConfigurator::configureAndWatch(const std::basic_string<UniChar>& filename, long delay)
{
        File file(filename);
#if APR_HAS_THREADS
        XMLWatchdog * xdog = new XMLWatchdog(file);
        xdog->setDelay(delay);
        xdog->start();
#else
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
#endif        
}
#endif

#if LOG4CXX_CFSTRING_API
void DOMConfigurator::configureAndWatch(const CFStringRef& filename, long delay)
{
        File file(filename);
#if APR_HAS_THREADS
        XMLWatchdog * xdog = new XMLWatchdog(file);
        xdog->setDelay(delay);
        xdog->start();
#else
    DOMConfigurator().doConfigure(file, LogManager::getLoggerRepository());
#endif        
}
#endif

void DOMConfigurator::parse(
                            Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* element,
                            apr_xml_doc* doc,
                            AppenderMap& appenders)
{
    std::string rootElementName(element->name);

    if (rootElementName != CONFIGURATION_TAG)
        {
                if(rootElementName == OLD_CONFIGURATION_TAG)
                {
                        //LogLog::warn(LOG4CXX_STR("The <")+String(OLD_CONFIGURATION_TAG)+
                        // LOG4CXX_STR("> element has been deprecated."));
                        //LogLog::warn(LOG4CXX_STR("Use the <")+String(CONFIGURATION_TAG)+
                        // LOG4CXX_STR("> element instead."));
                }
                else
                {
                        LogLog::error(LOG4CXX_STR("DOM element is - not a <configuration> element."));
                        return;
                }
    }

    LogString debugAttrib = subst(getAttribute(utf8Decoder, element, INTERNAL_DEBUG_ATTR));

    static const LogString NuLL(LOG4CXX_STR("NULL"));
    LogLog::debug(LOG4CXX_STR("debug attribute= \"") + debugAttrib +LOG4CXX_STR("\"."));
    // if the log4j.dtd is not specified in the XML file, then the
    // "debug" attribute is returned as the empty string.
    if(!debugAttrib.empty() && debugAttrib != NuLL)
        {
                LogLog::setInternalDebugging(OptionConverter::toBoolean(debugAttrib, true));
    }
        else
        {
                LogLog::debug(LOG4CXX_STR("Ignoring internalDebug attribute."));
    }


    LogString confDebug = subst(getAttribute(utf8Decoder, element, CONFIG_DEBUG_ATTR));
    if(!confDebug.empty() && confDebug != NuLL)
        {
                LogLog::warn(LOG4CXX_STR("The \"configDebug\" attribute is deprecated."));
                LogLog::warn(LOG4CXX_STR("Use the \"internalDebug\" attribute instead."));
                LogLog::setInternalDebugging(OptionConverter::toBoolean(confDebug, true));
    }

    LogString thresholdStr = subst(getAttribute(utf8Decoder, element, THRESHOLD_ATTR));
    LogLog::debug(LOG4CXX_STR("Threshold =\"") + thresholdStr +LOG4CXX_STR("\"."));
    if(!thresholdStr.empty() && thresholdStr != NuLL)
        {
                repository->setThreshold(thresholdStr);
    }

    apr_xml_elem* currentElement;
    for(currentElement = element->first_child;
        currentElement;
        currentElement = currentElement->next) {
                        std::string tagName(currentElement->name);

                        if (tagName == CATEGORY_FACTORY_TAG)
                        {
                                parseLoggerFactory(p, utf8Decoder, currentElement);
                        }
    }

    for(currentElement = element->first_child;
        currentElement;
        currentElement = currentElement->next) {
                        std::string tagName(currentElement->name);

                        if (tagName == CATEGORY || tagName == LOGGER)
                        {
                                parseLogger(p, utf8Decoder, currentElement, doc, appenders);
                        }
                        else if (tagName == ROOT_TAG)
                        {
                                parseRoot(p, utf8Decoder, currentElement, doc, appenders);
                        }
    }
}

LogString DOMConfigurator::subst(const LogString& value)
{
    try
        {
                return OptionConverter::substVars(value, props);
    }
        catch(IllegalArgumentException& e)
        {
                LogLog::warn(LOG4CXX_STR("Could not perform variable substitution."), e);
                return value;
    }
}


LogString DOMConfigurator::getAttribute(
                                        log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                        apr_xml_elem* element, 
                                        const std::string& attrName) {
    LogString attrValue;
    for(apr_xml_attr* attr = element->attr;
        attr;
        attr = attr->next) {
        if (attrName == attr->name) {
            ByteBuffer buf((char*) attr->value, strlen(attr->value));
            utf8Decoder->decode(buf, attrValue);
        }
    }
    return attrValue;
}
