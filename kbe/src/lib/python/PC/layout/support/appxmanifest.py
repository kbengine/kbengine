"""
File generation for APPX/MSIX manifests.
"""

__author__ = "Steve Dower <steve.dower@python.org>"
__version__ = "3.8"


import collections
import ctypes
import io
import os
import sys

from pathlib import Path, PureWindowsPath
from xml.etree import ElementTree as ET

from .constants import *

__all__ = []


def public(f):
    __all__.append(f.__name__)
    return f


APPX_DATA = dict(
    Name="PythonSoftwareFoundation.Python.{}".format(VER_DOT),
    Version="{}.{}.{}.0".format(VER_MAJOR, VER_MINOR, VER_FIELD3),
    Publisher=os.getenv(
        "APPX_DATA_PUBLISHER", "CN=4975D53F-AA7E-49A5-8B49-EA4FDC1BB66B"
    ),
    DisplayName="Python {}".format(VER_DOT),
    Description="The Python {} runtime and console.".format(VER_DOT),
    ProcessorArchitecture="x64" if IS_X64 else "x86",
)

PYTHON_VE_DATA = dict(
    DisplayName="Python {}".format(VER_DOT),
    Description="Python interactive console",
    Square150x150Logo="_resources/pythonx150.png",
    Square44x44Logo="_resources/pythonx44.png",
    BackgroundColor="transparent",
)

PYTHONW_VE_DATA = dict(
    DisplayName="Python {} (Windowed)".format(VER_DOT),
    Description="Python windowed app launcher",
    Square150x150Logo="_resources/pythonwx150.png",
    Square44x44Logo="_resources/pythonwx44.png",
    BackgroundColor="transparent",
    AppListEntry="none",
)

PIP_VE_DATA = dict(
    DisplayName="pip (Python {})".format(VER_DOT),
    Description="pip package manager for Python {}".format(VER_DOT),
    Square150x150Logo="_resources/pythonx150.png",
    Square44x44Logo="_resources/pythonx44.png",
    BackgroundColor="transparent",
    AppListEntry="none",
)

IDLE_VE_DATA = dict(
    DisplayName="IDLE (Python {})".format(VER_DOT),
    Description="IDLE editor for Python {}".format(VER_DOT),
    Square150x150Logo="_resources/pythonwx150.png",
    Square44x44Logo="_resources/pythonwx44.png",
    BackgroundColor="transparent",
)

APPXMANIFEST_NS = {
    "": "http://schemas.microsoft.com/appx/manifest/foundation/windows10",
    "m": "http://schemas.microsoft.com/appx/manifest/foundation/windows10",
    "uap": "http://schemas.microsoft.com/appx/manifest/uap/windows10",
    "rescap": "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities",
    "rescap4": "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/4",
    "desktop4": "http://schemas.microsoft.com/appx/manifest/desktop/windows10/4",
    "desktop6": "http://schemas.microsoft.com/appx/manifest/desktop/windows10/6",
    "uap3": "http://schemas.microsoft.com/appx/manifest/uap/windows10/3",
    "uap4": "http://schemas.microsoft.com/appx/manifest/uap/windows10/4",
    "uap5": "http://schemas.microsoft.com/appx/manifest/uap/windows10/5",
}

APPXMANIFEST_TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
    xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
    xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities"
    xmlns:rescap4="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/4"
    xmlns:desktop4="http://schemas.microsoft.com/appx/manifest/desktop/windows10/4"
    xmlns:uap4="http://schemas.microsoft.com/appx/manifest/uap/windows10/4"
    xmlns:uap5="http://schemas.microsoft.com/appx/manifest/uap/windows10/5">
    <Identity Name=""
              Version=""
              Publisher=""
              ProcessorArchitecture="" />
    <Properties>
        <DisplayName></DisplayName>
        <PublisherDisplayName>Python Software Foundation</PublisherDisplayName>
        <Description></Description>
        <Logo>_resources/pythonx50.png</Logo>
    </Properties>
    <Resources>
        <Resource Language="en-US" />
    </Resources>
    <Dependencies>
        <TargetDeviceFamily Name="Windows.Desktop" MinVersion="10.0.17763.0" MaxVersionTested="" />
    </Dependencies>
    <Capabilities>
        <rescap:Capability Name="runFullTrust"/>
    </Capabilities>
    <Applications>
    </Applications>
    <Extensions>
    </Extensions>
</Package>"""


RESOURCES_XML_TEMPLATE = r"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!--This file is input for makepri.exe. It should be excluded from the final package.-->
<resources targetOsVersion="10.0.0" majorVersion="1">
    <packaging>
        <autoResourcePackage qualifier="Language"/>
        <autoResourcePackage qualifier="Scale"/>
        <autoResourcePackage qualifier="DXFeatureLevel"/>
    </packaging>
    <index root="\" startIndexAt="\">
        <default>
            <qualifier name="Language" value="en-US"/>
            <qualifier name="Contrast" value="standard"/>
            <qualifier name="Scale" value="100"/>
            <qualifier name="HomeRegion" value="001"/>
            <qualifier name="TargetSize" value="256"/>
            <qualifier name="LayoutDirection" value="LTR"/>
            <qualifier name="Theme" value="dark"/>
            <qualifier name="AlternateForm" value=""/>
            <qualifier name="DXFeatureLevel" value="DX9"/>
            <qualifier name="Configuration" value=""/>
            <qualifier name="DeviceFamily" value="Universal"/>
            <qualifier name="Custom" value=""/>
        </default>
        <indexer-config type="folder" foldernameAsQualifier="true" filenameAsQualifier="true" qualifierDelimiter="$"/>
        <indexer-config type="resw" convertDotsToSlashes="true" initialPath=""/>
        <indexer-config type="resjson" initialPath=""/>
        <indexer-config type="PRI"/>
    </index>
</resources>"""


SCCD_FILENAME = "PC/classicAppCompat.sccd"

REGISTRY = {
    "HKCU\\Software\\Python\\PythonCore": {
        VER_DOT: {
            "DisplayName": APPX_DATA["DisplayName"],
            "SupportUrl": "https://www.python.org/",
            "SysArchitecture": "64bit" if IS_X64 else "32bit",
            "SysVersion": VER_DOT,
            "Version": "{}.{}.{}".format(VER_MAJOR, VER_MINOR, VER_MICRO),
            "InstallPath": {
                # I have no idea why the trailing spaces are needed, but they seem to be needed.
                "": "[{AppVPackageRoot}][                    ]",
                "ExecutablePath": "[{AppVPackageRoot}]python.exe[                    ]",
                "WindowedExecutablePath": "[{AppVPackageRoot}]pythonw.exe[                    ]",
            },
            "Help": {
                "Main Python Documentation": {
                    "_condition": lambda ns: ns.include_chm,
                    "": "[{{AppVPackageRoot}}]Doc\\{}[                    ]".format(
                        PYTHON_CHM_NAME
                    ),
                },
                "Local Python Documentation": {
                    "_condition": lambda ns: ns.include_html_doc,
                    "": "[{AppVPackageRoot}]Doc\\html\\index.html[                    ]",
                },
                "Online Python Documentation": {
                    "": "https://docs.python.org/{}".format(VER_DOT)
                },
            },
            "Idle": {
                "_condition": lambda ns: ns.include_idle,
                "": "[{AppVPackageRoot}]Lib\\idlelib\\idle.pyw[                    ]",
            },
        }
    }
}


def get_packagefamilyname(name, publisher_id):
    class PACKAGE_ID(ctypes.Structure):
        _fields_ = [
            ("reserved", ctypes.c_uint32),
            ("processorArchitecture", ctypes.c_uint32),
            ("version", ctypes.c_uint64),
            ("name", ctypes.c_wchar_p),
            ("publisher", ctypes.c_wchar_p),
            ("resourceId", ctypes.c_wchar_p),
            ("publisherId", ctypes.c_wchar_p),
        ]
        _pack_ = 4

    pid = PACKAGE_ID(0, 0, 0, name, publisher_id, None, None)
    result = ctypes.create_unicode_buffer(256)
    result_len = ctypes.c_uint32(256)
    r = ctypes.windll.kernel32.PackageFamilyNameFromId(
        pid, ctypes.byref(result_len), result
    )
    if r:
        raise OSError(r, "failed to get package family name")
    return result.value[: result_len.value]


def _fixup_sccd(ns, sccd, new_hash=None):
    if not new_hash:
        return sccd

    NS = dict(s="http://schemas.microsoft.com/appx/2016/sccd")
    with open(sccd, "rb") as f:
        xml = ET.parse(f)

    pfn = get_packagefamilyname(APPX_DATA["Name"], APPX_DATA["Publisher"])

    ae = xml.find("s:AuthorizedEntities", NS)
    ae.clear()

    e = ET.SubElement(ae, ET.QName(NS["s"], "AuthorizedEntity"))
    e.set("AppPackageFamilyName", pfn)
    e.set("CertificateSignatureHash", new_hash)

    for e in xml.findall("s:Catalog", NS):
        e.text = "FFFF"

    sccd = ns.temp / sccd.name
    sccd.parent.mkdir(parents=True, exist_ok=True)
    with open(sccd, "wb") as f:
        xml.write(f, encoding="utf-8")

    return sccd


@public
def get_appx_layout(ns):
    if not ns.include_appxmanifest:
        return

    yield "AppxManifest.xml", ns.temp / "AppxManifest.xml"
    yield "_resources.xml", ns.temp / "_resources.xml"
    icons = ns.source / "PC" / "icons"
    yield "_resources/pythonx44.png", icons / "pythonx44.png"
    yield "_resources/pythonx44$targetsize-44_altform-unplated.png", icons / "pythonx44.png"
    yield "_resources/pythonx50.png", icons / "pythonx50.png"
    yield "_resources/pythonx50$targetsize-50_altform-unplated.png", icons / "pythonx50.png"
    yield "_resources/pythonx150.png", icons / "pythonx150.png"
    yield "_resources/pythonx150$targetsize-150_altform-unplated.png", icons / "pythonx150.png"
    yield "_resources/pythonwx44.png", icons / "pythonwx44.png"
    yield "_resources/pythonwx44$targetsize-44_altform-unplated.png", icons / "pythonwx44.png"
    yield "_resources/pythonwx150.png", icons / "pythonwx150.png"
    yield "_resources/pythonwx150$targetsize-150_altform-unplated.png", icons / "pythonwx150.png"
    sccd = ns.source / SCCD_FILENAME
    if sccd.is_file():
        # This should only be set for side-loading purposes.
        sccd = _fixup_sccd(ns, sccd, os.getenv("APPX_DATA_SHA256"))
        yield sccd.name, sccd


def find_or_add(xml, element, attr=None, always_add=False):
    if always_add:
        e = None
    else:
        q = element
        if attr:
            q += "[@{}='{}']".format(*attr)
        e = xml.find(q, APPXMANIFEST_NS)
    if e is None:
        prefix, _, name = element.partition(":")
        name = ET.QName(APPXMANIFEST_NS[prefix or ""], name)
        e = ET.SubElement(xml, name)
        if attr:
            e.set(*attr)
    return e


def _get_app(xml, appid):
    if appid:
        app = xml.find(
            "m:Applications/m:Application[@Id='{}']".format(appid), APPXMANIFEST_NS
        )
        if app is None:
            raise LookupError(appid)
    else:
        app = xml
    return app


def add_visual(xml, appid, data):
    app = _get_app(xml, appid)
    e = find_or_add(app, "uap:VisualElements")
    for i in data.items():
        e.set(*i)
    return e


def add_alias(xml, appid, alias, subsystem="windows"):
    app = _get_app(xml, appid)
    e = find_or_add(app, "m:Extensions")
    e = find_or_add(e, "uap5:Extension", ("Category", "windows.appExecutionAlias"))
    e = find_or_add(e, "uap5:AppExecutionAlias")
    e.set(ET.QName(APPXMANIFEST_NS["desktop4"], "Subsystem"), subsystem)
    e = find_or_add(e, "uap5:ExecutionAlias", ("Alias", alias))


def add_file_type(xml, appid, name, suffix, parameters='"%1"'):
    app = _get_app(xml, appid)
    e = find_or_add(app, "m:Extensions")
    e = find_or_add(e, "uap3:Extension", ("Category", "windows.fileTypeAssociation"))
    e = find_or_add(e, "uap3:FileTypeAssociation", ("Name", name))
    e.set("Parameters", parameters)
    e = find_or_add(e, "uap:SupportedFileTypes")
    if isinstance(suffix, str):
        suffix = [suffix]
    for s in suffix:
        ET.SubElement(e, ET.QName(APPXMANIFEST_NS["uap"], "FileType")).text = s


def add_application(
    ns, xml, appid, executable, aliases, visual_element, subsystem, file_types
):
    node = xml.find("m:Applications", APPXMANIFEST_NS)
    suffix = "_d.exe" if ns.debug else ".exe"
    app = ET.SubElement(
        node,
        ET.QName(APPXMANIFEST_NS[""], "Application"),
        {
            "Id": appid,
            "Executable": executable + suffix,
            "EntryPoint": "Windows.FullTrustApplication",
            ET.QName(APPXMANIFEST_NS["desktop4"], "SupportsMultipleInstances"): "true",
        },
    )
    if visual_element:
        add_visual(app, None, visual_element)
    for alias in aliases:
        add_alias(app, None, alias + suffix, subsystem)
    if file_types:
        add_file_type(app, None, *file_types)
    return app


def _get_registry_entries(ns, root="", d=None):
    r = root if root else PureWindowsPath("")
    if d is None:
        d = REGISTRY
    for key, value in d.items():
        if key == "_condition":
            continue
        elif isinstance(value, dict):
            cond = value.get("_condition")
            if cond and not cond(ns):
                continue
            fullkey = r
            for part in PureWindowsPath(key).parts:
                fullkey /= part
                if len(fullkey.parts) > 1:
                    yield str(fullkey), None, None
            yield from _get_registry_entries(ns, fullkey, value)
        elif len(r.parts) > 1:
            yield str(r), key, value


def add_registry_entries(ns, xml):
    e = find_or_add(xml, "m:Extensions")
    e = find_or_add(e, "rescap4:Extension")
    e.set("Category", "windows.classicAppCompatKeys")
    e.set("EntryPoint", "Windows.FullTrustApplication")
    e = ET.SubElement(e, ET.QName(APPXMANIFEST_NS["rescap4"], "ClassicAppCompatKeys"))
    for name, valuename, value in _get_registry_entries(ns):
        k = ET.SubElement(
            e, ET.QName(APPXMANIFEST_NS["rescap4"], "ClassicAppCompatKey")
        )
        k.set("Name", name)
        if value:
            k.set("ValueName", valuename)
            k.set("Value", value)
            k.set("ValueType", "REG_SZ")


def disable_registry_virtualization(xml):
    e = find_or_add(xml, "m:Properties")
    e = find_or_add(e, "desktop6:RegistryWriteVirtualization")
    e.text = "disabled"
    e = find_or_add(xml, "m:Capabilities")
    e = find_or_add(e, "rescap:Capability", ("Name", "unvirtualizedResources"))


@public
def get_appxmanifest(ns):
    for k, v in APPXMANIFEST_NS.items():
        ET.register_namespace(k, v)
    ET.register_namespace("", APPXMANIFEST_NS["m"])

    xml = ET.parse(io.StringIO(APPXMANIFEST_TEMPLATE))
    NS = APPXMANIFEST_NS
    QN = ET.QName

    node = xml.find("m:Identity", NS)
    for k in node.keys():
        value = APPX_DATA.get(k)
        if value:
            node.set(k, value)

    for node in xml.find("m:Properties", NS):
        value = APPX_DATA.get(node.tag.rpartition("}")[2])
        if value:
            node.text = value

    winver = sys.getwindowsversion()[:3]
    if winver < (10, 0, 17763):
        winver = 10, 0, 17763
    find_or_add(xml, "m:Dependencies/m:TargetDeviceFamily").set(
        "MaxVersionTested", "{}.{}.{}.0".format(*winver)
    )

    if winver > (10, 0, 17763):
        disable_registry_virtualization(xml)

    app = add_application(
        ns,
        xml,
        "Python",
        "python",
        ["python", "python{}".format(VER_MAJOR), "python{}".format(VER_DOT)],
        PYTHON_VE_DATA,
        "console",
        ("python.file", [".py"]),
    )

    add_application(
        ns,
        xml,
        "PythonW",
        "pythonw",
        ["pythonw", "pythonw{}".format(VER_MAJOR), "pythonw{}".format(VER_DOT)],
        PYTHONW_VE_DATA,
        "windows",
        ("python.windowedfile", [".pyw"]),
    )

    if ns.include_pip and ns.include_launchers:
        add_application(
            ns,
            xml,
            "Pip",
            "pip",
            ["pip", "pip{}".format(VER_MAJOR), "pip{}".format(VER_DOT)],
            PIP_VE_DATA,
            "console",
            ("python.wheel", [".whl"], 'install "%1"'),
        )

    if ns.include_idle and ns.include_launchers:
        add_application(
            ns,
            xml,
            "Idle",
            "idle",
            ["idle", "idle{}".format(VER_MAJOR), "idle{}".format(VER_DOT)],
            IDLE_VE_DATA,
            "windows",
            None,
        )

    if (ns.source / SCCD_FILENAME).is_file():
        add_registry_entries(ns, xml)
        node = xml.find("m:Capabilities", NS)
        node = ET.SubElement(node, QN(NS["uap4"], "CustomCapability"))
        node.set("Name", "Microsoft.classicAppCompat_8wekyb3d8bbwe")

    buffer = io.BytesIO()
    xml.write(buffer, encoding="utf-8", xml_declaration=True)
    return buffer.getbuffer()


@public
def get_resources_xml(ns):
    return RESOURCES_XML_TEMPLATE.encode("utf-8")
