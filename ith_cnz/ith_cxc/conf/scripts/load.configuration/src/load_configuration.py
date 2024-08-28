'''
Created on Jun 24, 2016

@author: estevol
'''
#!/usr/bin/env python

import yaml
import sys
import subprocess
import xml.etree.ElementTree as ET
import itertools
import os.path


REMOTE_SESSION=False

INCLUDES='includes'
PARAMETERS='parameters'
RESOURCES='resources'
GET_PARAM='get_param'

M_APZ="APZ"
M_APT="APT"

S_LEFT="LEFT"
S_RIGHT="RIGHT"

P_CONTROL="CONTROL"
P_TRANSPORT="TRANSPORT"

CONTROL_POSTFIX="LS"
TRANSPORT_POSTFIX="HS"

SLOT_LEFT="0"
SLOT_RIGHT="25"

TYPE_ATTRIBUTE="type"
MAGAZINE_ATTRIBUTE="magazine"
SIDE_ATTRIBUTE="side"
PLANE_ATTRIBUTE="plane"

VLAN_TYPE="VLAN"
SUBNETVLAN_TYPE="SUBNETVLAN"
ROUTER_TYPE="ROUTER"
INTERFACE_TYPE="INTERFACE"
ADDRESS_TYPE="ADDRESS"
STATIC_ROUTE_TYPE="STATIC_ROUTE"
DST_TYPE="DST"
NEXTHOP_TYPE="NEXTHOP"
VRRP_INTERFACE_TYPE="VRRP_INTERFACE"
VRRP_SESSION_TYPE="VRRP_SESSION"
BFD_PROFILE_TYPE="BFD_PROFILE"
PORT_TYPE="PORT"
ACL_TYPE="ACL"


HELLO="""<hello xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
<capabilities>
      <capability>urn:ietf:params:netconf:base:1.0</capability>
      <capability>urn:com:ericsson:ebase:0.1.0</capability>
      <capability>urn:com:ericsson:ebase:1.1.0</capability>
      <capability>urn:com:ericsson:ebase:1.2.0</capability>
      <capability>urn:ietf:params:netconf:capability:writable-running:1.0</capability>
      <capability>urn:ietf:params:netconf:capability:rollback-on-error:1.0</capability>
      <capability>urn:ietf:params:netconf:capability:notification:1.0</capability>
      <capability>urn:ericsson:com:netconf:action:1.0</capability>
      <capability>urn:ericsson:com:netconf:heartbeat:1.0</capability>
      <capability>urn:com:ericsson:netconf:operation:1.0</capability>
   </capabilities>
   </hello>]]>]]>"""
   
CLOSE_SESSION="<rpc message-id=\"2\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"><close-session/></rpc>]]>]]>"

MAPPING_DICT={}

MOM_NAMESPACE="{Urn:Com:Ericsson:Ecim:Axeinfrastructuretransportm}"

IPV4_SUBNETVLAN_STRUCT = MOM_NAMESPACE + "Ipv4Subnetvlan";  MAPPING_DICT[IPV4_SUBNETVLAN_STRUCT] = SUBNETVLAN_TYPE
IPV4_SUBNET_ATTR = MOM_NAMESPACE + "Ipv4Subnet";            MAPPING_DICT[IPV4_SUBNET_ATTR] = "ipv4subnet"
VLANID_ATTR = MOM_NAMESPACE + "Vlanid";                     MAPPING_DICT[VLANID_ATTR] = "vid"

INTERFACE_CLASS = MOM_NAMESPACE + "Interfaceipv4";          MAPPING_DICT[INTERFACE_CLASS] = INTERFACE_TYPE
ADMIN_STATE_ATTR = MOM_NAMESPACE + "Adminstate";            MAPPING_DICT[ADMIN_STATE_ATTR] = "admin_state"
ENCAPSULATION_ATTR = MOM_NAMESPACE + "Encapsulation";       MAPPING_DICT[ENCAPSULATION_ATTR] = "encapsulation_dn"

ADDRESS_CLASS = MOM_NAMESPACE + "Addressipv4";              MAPPING_DICT[ADDRESS_CLASS] = ADDRESS_TYPE
ADDRESS_ID_ATTR = MOM_NAMESPACE + "Addressipv4Id";          MAPPING_DICT[ADDRESS_ID_ATTR] = "name"
ADDRESS_ATTR = MOM_NAMESPACE + "Address";                   MAPPING_DICT[ADDRESS_ATTR] = "address"

VRRP_SESSION_CLASS = MOM_NAMESPACE + "Vrrpv3Session";       MAPPING_DICT[VRRP_SESSION_CLASS] = VRRP_SESSION_TYPE
VRRP_SESS_ID_ATTR = MOM_NAMESPACE + "Vrrpv3Sessionid";      MAPPING_DICT[VRRP_SESS_ID_ATTR] = "name"
PRIO_SESS_ATTR = MOM_NAMESPACE + "Prioritizedsession";      MAPPING_DICT[PRIO_SESS_ATTR] = "vrrp_prioritized_session"       
VRRP_IF_REF_ATTR = MOM_NAMESPACE + "Vrrpv3Interfaceref";    MAPPING_DICT[VRRP_IF_REF_ATTR] = "vrrp_if_name"

VRRP_IF_CLASS = MOM_NAMESPACE + "Vrrpv3Interface";          MAPPING_DICT[VRRP_IF_CLASS] = VRRP_INTERFACE_TYPE
VRRP_IF_ID_ATTR = MOM_NAMESPACE + "Vrrpv3Interfaceid";      MAPPING_DICT[VRRP_IF_ID_ATTR] = "name"
VRRP_VR_ID_ATTR = MOM_NAMESPACE + "Vridentity";             MAPPING_DICT[VRRP_VR_ID_ATTR] = "vr_id"

DST_CLASS = MOM_NAMESPACE + "Dst";                          MAPPING_DICT[DST_CLASS] = DST_TYPE
DST_ID_ATTR = MOM_NAMESPACE + "Dstid";                      MAPPING_DICT[DST_ID_ATTR] = "name"
DST_ATTR = MOM_NAMESPACE + "Dst";                           MAPPING_DICT[DST_ATTR] = "dst"

NEXTHOP_CLASS = MOM_NAMESPACE + "Nexthop";                  MAPPING_DICT[NEXTHOP_CLASS] = NEXTHOP_TYPE
NEXTHOP_ID_ATTR = MOM_NAMESPACE + "Nexthopid";              MAPPING_DICT[NEXTHOP_ID_ATTR] = "name"
ADM_DISTANCE_ATTR = MOM_NAMESPACE + "Admindistance";        MAPPING_DICT[ADM_DISTANCE_ATTR] = "admin_distance"
DISCARD_ATTR = MOM_NAMESPACE + "Discard";                   MAPPING_DICT[DISCARD_ATTR] = "discard"

ME_ID_ATTR="{Urn:Com:Ericsson:Ecim:Comtop}Managedelementid"; MAPPING_DICT[DISCARD_ATTR] = "me_id"


#REMOTE_SESSION_CMD=["ssh", "%s@%s" % (USERNAME, HOST), "-x", "-p %s" % PORT]
LOCAL_SESSION_CMD=["/opt/com/bin/netconf"]

# Utilities
def convert_plane(func):
    def wrapper(*args, **kwargs):
        output = []
        plane_list = flat_list([func(*args, **kwargs)])
        for item in plane_list:
            plane_postfix = get_plane_postfix(item)
            if plane_postfix != None: 
                output += [plane_postfix]
        return output
        
    return wrapper

def convert_magazine(func):
    def wrapper(*args, **kwargs):
        output = []
        magazine_list = flat_list([func(*args, **kwargs)])
        for item in magazine_list:
            address = get_mag_address(item)
            if address != None:
                output += [address]
        return output
    return wrapper

def convert_side(func):
    def wrapper(*args, **kwargs):
        output = []
        side_list = flat_list([func(*args, **kwargs)]) 
        for item in side_list:
            slot = get_slot(item)
            if slot != None:
                output += [slot]
        return output        
    return wrapper

def get_plane_postfix (plane):
    plane_postfix=None
    if plane == P_CONTROL:
        plane_postfix = CONTROL_POSTFIX
    elif plane == P_TRANSPORT:
        plane_postfix = TRANSPORT_POSTFIX

    return plane_postfix


def get_mag_address (magazine):
    mag_address=None

    if magazine == M_APZ:
        mag_address = APZ_MAG
    elif magazine == M_APT:
        mag_address = APT_MAG

    return mag_address

def get_slot (side):
    slot=None

    if side == S_LEFT:
        slot = SLOT_LEFT
    elif side == S_RIGHT:
        slot = SLOT_RIGHT

    return slot

def build_bridge_id(mag_address, slot, plane):
    return mag_address + "_" + str(slot) + "_" + plane

def build_router_id(mag_address, slot, router_name):
    return mag_address + "_" + str(slot) + ":" + router_name

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    
def convert_attr_name(attr_name):
    try:
        return MAPPING_DICT[attr_name]
    except:
        return attr_name
        

class Resource:
    def __init__(self, tree, parent, resource_id):
        self.id = resource_id
        self.attributes = {}
        for attribute in parent[resource_id]:
            self.attributes[attribute] = get_attribute_value(tree, parent, resource_id, attribute)
    
    @classmethod        
    def from_netconf_struct(cls, struct_name, struct):
        tree = {}
        resource = {"type" : convert_attr_name(struct_name)}
        tree[RESOURCES] = {"" : resource}
        for key in struct:
            tree[RESOURCES][""][convert_attr_name(key)] = struct[key]
            
        return cls(tree, tree[RESOURCES], "")
          

    def get_attribute_names(self):
        return self.attributes.keys()

    def get_attribute ( self, attrName ):
        try:
            return self.attributes[attrName]
        except:
            return None
        
    def get_type( self ):
        return self.get_attribute(TYPE_ATTRIBUTE)
    
    @convert_plane
    def get_plane( self ):
        return self.get_attribute(PLANE_ATTRIBUTE)
    
    @convert_magazine
    def get_mag_address(self):
        return self.get_attribute(MAGAZINE_ATTRIBUTE)
    
    @convert_side
    def get_slot(self):
        return self.get_attribute(SIDE_ATTRIBUTE)
    
    def print_object(self):
        print bcolors.HEADER + "Object " + str(self.get_type()) + " " + str(self.id) + bcolors.ENDC
        self.print_attributes()
        print ""
    
    def print_attributes( self ):
        for attr_name in self.get_attribute_names():
            if attr_name != TYPE_ATTRIBUTE:
                print str(attr_name) + " = " + str(self.get_attribute(attr_name))
                    
                    
####################################################################################################################################################################
######## YAML Parsing methods ######################################################################################################################################
####################################################################################################################################################################

def load_yml (doc, file_list):   
    for file_name in file_list:
        new_doc = yaml.load(open(file_name, 'r'))
        root =  os.path.split(file_name)[0]    
        
        #Load includes first
        for included_file in new_doc.get(INCLUDES, [] ):
            included_file_path = os.path.join(root, included_file)
            load_yml(doc, flat_list(included_file_path))
        
        doc[PARAMETERS].update(new_doc.get(PARAMETERS, []))
        doc[RESOURCES].update(new_doc.get(RESOURCES, []))
    

def flatten(output, list_of_values):
    if isinstance(list_of_values, list):
        for item in list_of_values:
            flatten(output, item)
    else:
        output += [list_of_values]

def flat_list(list_of_values):
    output = []
    flatten(output, list_of_values)
    return output
        

def get_parameter ( tree, paramName ):    
    if isinstance(paramName, basestring) or isinstance(paramName, int):
        paramValue = get_parameter_value ( tree, paramName )
    elif isinstance(paramName, dict):
        try:
            nestedParamName = get_value( tree, paramName[GET_PARAM] )
            paramValue = get_parameter( tree, nestedParamName )
        except:
            #Parameter is a dictionary
            paramValue = {}
            for key in paramName:
                paramValue[get_value(tree, key)] = get_value(tree, paramName[key])
    else:
        paramValue = []
        for item in paramName:
            paramValue += [get_parameter( tree, item )]

    return get_value(tree, paramValue)


def get_value ( tree, value ):
    if isinstance(value, basestring) or isinstance(value, int) or None == value:
        if ("None" == value) or (None == value):
            output = None
        else:
            output = str(value)
    elif isinstance(value, dict):
        try:
            paramName = get_value( tree, value[GET_PARAM] )
            output = get_parameter( tree, paramName )
        except:
            #Attribute is a dictionary
            output = {}
            for key in value:
                output[get_value(tree, key)] = get_value(tree, value[key])
    else:
        output = []
        for item in value:
            output += [get_value( tree, item )]

    if isinstance(output, list):
        return flat_list(output)
    else:
        return output

def get_attribute_value ( tree, parent, objName, attrName ):
    try:
        return get_value(tree, parent[objName][attrName])
    except:
        return None

def get_parameter_value ( tree, paramName ):
    try:
        return get_value(tree, tree[PARAMETERS][paramName])
    except:
        return None



####################################################################################################################################################################
######## NETCONF session methods ###################################################################################################################################
####################################################################################################################################################################

def debug_cmd(func):
    def debug(cmd):
        print ""
        print "COMMAND SENT:"
        print cmd
        return ""
    
    return debug

def debug_and_execute_cmd(func):
    def debug_and_execute(cmd):
        debug_cmd(func)(cmd)
        result = func(cmd)
        print""
        print "COMMAND RESULT:"
        print result
        return result
    return debug_and_execute

@debug_cmd
def send_netconf_command ( command ):

    session = netconf_session()

    query = HELLO + command + CLOSE_SESSION
    cmd_out = session.communicate(query)

    return cmd_out[0]

def netconf_session():
    
    if REMOTE_SESSION:
        session_cmd = ["ssh", "%s@%s" % (USERNAME, HOST), "-x", "-p %s" % PORT]
    else:
        session_cmd = LOCAL_SESSION_CMD
    
    return subprocess.Popen(session_cmd,
                            shell=False,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)


def netconf_get(netconf_query, attribute):
    reply = send_netconf_command (netconf_query)
    reply = reply.split("]]>]]>")

    for item in reply:
        item = item.split("?>")
        if len(item) > 1:
            tree = ET.fromstring(item[1])
            attrValue = find_attribute(tree, attribute)
            if attrValue != None:
                return attrValue
    return None

def netconf_get_struct(netconf_query, structName, attrList):
    reply = send_netconf_command (netconf_query)
    reply = reply.split("]]>]]>")

    #print "STRUCT_NAME " + structName
    #print reply
    
    for item in reply:
        item = item.split("?>")
        if len(item) > 1:
            tree = ET.fromstring(item[1])
            attrValue = find_struct(tree, structName, attrList)
            if attrValue != None:
                return attrValue
    return []

def netconf_edit_config(netconf_query):
    reply = send_netconf_command (netconf_query)
    reply = reply.split("]]>]]>")
    error = 0

    for item in reply:
        item = item.split("?>")
        if len(item) > 1:
            tree = ET.fromstring(item[1])           
            error1 = find_struct(tree, "{Urn:Ietf:Params:Xml:Ns:Netconf:Base:1.0}Rpc-Error", ["{Urn:Ietf:Params:Xml:Ns:Netconf:Base:1.0}Error-Severity"])
            error2 = find_struct(tree, "Rpc-Error", ["Error-Severity"])
            if error1 != None:
                #print error1
                error+=1
            if error2 != None:
                #print error2
                error+=1
    return error

def execute_netconf_edit_config(func):
    def execute(*args, **kwargs):
        if netconf_edit_config(func(*args, **kwargs)) == 0:
            sys.stdout.write(bcolors.OKGREEN + "SUCCESS" + bcolors.ENDC)
        else:
            sys.stdout.write(bcolors.FAIL + "FAILURE" + bcolors.ENDC)
        print ""
    
    return execute

def execute_netconf_get_attribute(attrName):
    def wrapper(func):
        def execute(*args, **kwargs):
            return netconf_get(func(*args, **kwargs), attrName)
        return execute
    return wrapper

def execute_netconf_get_struct(structName, attrList):
    def wrapper(func):
        def execute(*args, **kwargs):
            return netconf_get_struct(func(*args, **kwargs), structName, attrList)
        return execute
    return wrapper

def find_attribute(root, attrName):
    #print root.tag.title() + attrName
    if root.tag.title() == attrName:
        value = root.attrib.get('name', root.text)        
        if None == value:
            return "" #Empty attribute
        else:
            return value
    else:
        for elem in root.getchildren():
            value = find_attribute(elem, attrName)
            if value != None:
                return value  
    
    return None

def find_struct(root, structName, attributeList):
    value = []
    #print root.tag.title() + " " + structName
    if root.tag.title() == structName:
        out = {}
        for item in attributeList:
            out[item] = find_attribute(root, item)
        if len(out) > 0:
            return Resource.from_netconf_struct(structName, out)
    else:
        for elem in root.getchildren():
            struct = find_struct(elem, structName, attributeList)
            if struct != None:
                value.extend([struct])
                
        if len(value) > 0:
            return flat_list(value)

    return None

####################################################################################################################################################################
######## NETCONF utilities methods #################################################################################################################################
####################################################################################################################################################################

def build_netconf_attribute(attr_name, attr_value):
    
    # None:         attribute unset
    # Empty:        attribute to delete
    # String:       valid attribute
    
    if (attr_value == None):
        return ""
    elif (len(attr_value) == 0):
        return  "<" + attr_name + " xc:operation=\"delete\">" + "</" + attr_name + ">"
    else:
        if isinstance(attr_value, list):
            out = ""
            for item in attr_value:
                out += build_netconf_attribute(attr_name, item)
            return out
        else:
            return "<" + attr_name + ">" + attr_value + "</" + attr_name + ">"

def build_subnet_vlan_struct(subnet, vlan_id):
    try:
        return build_netconf_attribute("ipv4SubnetVlan", build_netconf_attribute("vlanId",vlan_id) + build_netconf_attribute("ipv4Subnet",subnet) )
    except:
        return ""
    
def build_multiple_dn(argName):
    def wrapper(func):
        def execute(*args, **kwargs):
            if argName in kwargs and isinstance(kwargs[argName], list):
                arg_list = kwargs[argName]
                out = []
                for x in arg_list:
                    kwargs[argName] = x
                    out += [func(*args, **kwargs)]
                return out
            else:
                return func(*args, **kwargs)
        return execute
    return wrapper

@build_multiple_dn("bfd_profile")
def build_bfd_profile_dn ( me_id, router_id,  bfd_profile):
    try:
        return "ManagedElement=""" + me_id + """,SystemFunctions=1,AxeFunctions=1,SystemComponentHandling=1,TransportM=1,Router=""" + router_id + """,BfdProfile=""" + bfd_profile
    except:
        return None
    
@build_multiple_dn("acl_name")
def build_acl_dn ( me_id, router_id,  acl_name):
    try:
        return "ManagedElement=""" + me_id + """,SystemFunctions=1,AxeFunctions=1,SystemComponentHandling=1,TransportM=1,Router=""" + router_id + """,AclIpv4=""" + acl_name
    except:
        return None
    
@build_multiple_dn("port_id")
def build_port_dn ( me_id, bridge_id,  port_id):        
    try:
        return "ManagedElement=""" + me_id + """,SystemFunctions=1,AxeFunctions=1,SystemComponentHandling=1,TransportM=1,Bridge=""" + bridge_id + """,BridgePort=""" + port_id
    except:
        return None

@build_multiple_dn("vlan_name")
def build_vlan_dn ( me_id, bridge_id,  vlan_name):
    try:
        return "ManagedElement=" + me_id + ",SystemFunctions=1,AxeFunctions=1,SystemComponentHandling=1,TransportM=1,Bridge=" + bridge_id + ",Vlan=" + vlan_name
    except:
        return None

@build_multiple_dn("vrrp_if_name")
def build_vrrp_interface_dn ( me_id, vrrp_if_name):
    try:
        return "ManagedElement=""" + me_id + """,SystemFunctions=1,AxeFunctions=1,SystemComponentHandling=1,TransportM=1,Vrrpv3Interface=""" + vrrp_if_name
    except:
        return None


####################################################################################################################################################################
######## NETCONF get queries #######################################################################################################################################
####################################################################################################################################################################


@execute_netconf_get_attribute(ME_ID_ATTR)
def get_me_id():
    
    return """
    <rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
        <get>
            <filter type=\"subtree\">
                <ManagedElement>
                    <managedElementId/>
                </ManagedElement>
            </filter>
        </get>
    </rpc>
    ]]>]]>"""

@execute_netconf_get_struct(IPV4_SUBNETVLAN_STRUCT, [IPV4_SUBNET_ATTR, VLANID_ATTR])
def get_subnet_vlan(me_id, bridge_id):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Bridge>
    """ + build_netconf_attribute("bridgeId", bridge_id) + """
    <ipv4SubnetVlan/>
    </Bridge>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query

@execute_netconf_get_struct(INTERFACE_CLASS, [ADMIN_STATE_ATTR, ENCAPSULATION_ATTR])
def get_interface(me_id, router_id, interface_id):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <InterfaceIPv4>
    """ + build_netconf_attribute("interfaceIPv4Id", interface_id) + """
    </InterfaceIPv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query

@execute_netconf_get_struct(ADDRESS_CLASS, [ADDRESS_ATTR, ADDRESS_ID_ATTR])
def get_interface_address(me_id, router_id, interface_id):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <InterfaceIPv4>
    """ + build_netconf_attribute("interfaceIPv4Id", interface_id) + """
    <AddressIPv4/>
    </InterfaceIPv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query

@execute_netconf_get_struct(VRRP_IF_CLASS, [VRRP_IF_ID_ATTR, VRRP_VR_ID_ATTR])
def get_vrrp_interface(me_id, vrrp_interface_id):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Vrrpv3Interface>
    """ + build_netconf_attribute("vrrpv3InterfaceId", vrrp_interface_id) + """
    </Vrrpv3Interface>    
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query

@execute_netconf_get_struct(ADDRESS_CLASS, [ADDRESS_ATTR, ADDRESS_ID_ATTR])
def get_vrrp_address(me_id, vrrp_interface_id):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Vrrpv3Interface>
    """ + build_netconf_attribute("vrrpv3InterfaceId", vrrp_interface_id) + """
    <AddressIPv4/>
    </Vrrpv3Interface>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query

@execute_netconf_get_struct(VRRP_SESSION_CLASS, [VRRP_SESS_ID_ATTR, PRIO_SESS_ATTR, VRRP_IF_REF_ATTR])
def get_vrrp_session(me_id, router_id, interface_id):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <InterfaceIPv4>
    """ + build_netconf_attribute("interfaceIPv4Id", interface_id) + """
    <Vrrpv3Session/>
    </InterfaceIPv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query


@execute_netconf_get_struct(DST_CLASS, [DST_ATTR, DST_ID_ATTR])
def get_dst(me_id, router_id, dst):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <RouteTableIPv4Static>
    <routeTableIPv4StaticId>1</routeTableIPv4StaticId>
    <Dst>
    """ + build_netconf_attribute("dst", dst) + """
    </Dst>
    </RouteTableIPv4Static>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query

@execute_netconf_get_struct(NEXTHOP_CLASS, [ADDRESS_ATTR, ADM_DISTANCE_ATTR, NEXTHOP_ID_ATTR, DISCARD_ATTR])
def get_nexthop(me_id, router_id, dst_id):    
    query = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <get>
    <filter type=\"subtree\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <RouteTableIPv4Static>
    <routeTableIPv4StaticId>1</routeTableIPv4StaticId>
    <Dst>
    """ + build_netconf_attribute("dstId", dst_id) + """
    <NextHop/>
    </Dst>
    </RouteTableIPv4Static>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </filter>
    </get>
    </rpc>
    ]]>]]>""" 
    return query


####################################################################################################################################################################
######## NETCONF edit-config queries ###############################################################################################################################
####################################################################################################################################################################

@execute_netconf_edit_config
def set_port (me_id, bridge_id, port_id, admin_state, user_label = None):   
    query = """
    <rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Bridge>
    """ + build_netconf_attribute("bridgeId", bridge_id) + """
    <BridgePort>
    """ + build_netconf_attribute("bridgePortId", port_id) + """
    """ + build_netconf_attribute("adminState", admin_state) + """
    """ + build_netconf_attribute("userLabel", user_label) + """
    </BridgePort>
    </Bridge>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
        
    return query


@execute_netconf_edit_config
def create_vlan (me_id, bridge_id, vlan_name, vlan_id, tagged_bridge_ports, untagged_bridge_ports, user_label = None):   
    query = """
    <rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Bridge>
    """ + build_netconf_attribute("bridgeId", bridge_id) + """
    <Vlan>
    """ + build_netconf_attribute("vlanId", vlan_name) + """
    """ + build_netconf_attribute("vid", vlan_id) + """
    """ + build_netconf_attribute("taggedBridgePorts", tagged_bridge_ports) + """
    """ + build_netconf_attribute("untaggedBridgePorts", untagged_bridge_ports) + """
    """ + build_netconf_attribute("userLabel", user_label) + """
    </Vlan>
    </Bridge>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
        
    return query

@execute_netconf_edit_config
def create_subnetvlan(me_id, bridge_id, subnet_struct):
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Bridge>
    """ + build_netconf_attribute("bridgeId", bridge_id) + """
    """ + subnet_struct + """
    </Bridge>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_router(me_id, router_id, user_label = None):
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    """ + build_netconf_attribute("userLabel", user_label) + """
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_bfd_profile(me_id, router_id, profile_id, det_multiplier, int_rx_min_required, int_tx_min_desired):
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <BfdProfile>
    """ + build_netconf_attribute("bfdProfileId", profile_id) + """
    """ + build_netconf_attribute("detectionMultiplier", det_multiplier) + """
    """ + build_netconf_attribute("intervalRxMinRequired", int_rx_min_required) + """
    """ + build_netconf_attribute("intervalTxMinDesired", int_tx_min_desired) + """
    </BfdProfile>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_interface(me_id, router_id, interface_id, admin_state, encapsulation_dn, mtu, bfd_profile, acl_ingress_dn, user_label = None):
    
    if (bfd_profile == None):
        bfd_profile_ref = build_netconf_attribute("bfdStaticRoutes", "DISABLED") + build_netconf_attribute("bfdProfile", None)
    else:
        bfd_profile_ref = build_netconf_attribute("bfdStaticRoutes", "ENABLED") + build_netconf_attribute("bfdProfile", build_bfd_profile_dn ( me_id, router_id,  bfd_profile = bfd_profile) )
    
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <InterfaceIPv4>
    """ + build_netconf_attribute("interfaceIPv4Id", interface_id) + """
    """ + build_netconf_attribute("encapsulation", encapsulation_dn) + """
    """ + build_netconf_attribute("adminState", admin_state) + """
    """ + build_netconf_attribute("mtu", mtu) + """
    """ + bfd_profile_ref + """
    """ + build_netconf_attribute("aclIngress", acl_ingress_dn) + """
    """ + build_netconf_attribute("userLabel", user_label) + """
    </InterfaceIPv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_address(me_id, router_id, interface_id, address_id, address):
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <InterfaceIPv4>
    """ + build_netconf_attribute("interfaceIPv4Id", interface_id) + """
    <AddressIPv4>
    """ + build_netconf_attribute("addressIPv4Id", address_id) + """
    """ + build_netconf_attribute("address", address) + """
    </AddressIPv4>
    </InterfaceIPv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_dst(me_id, router_id, dst_id, dst):
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <RouteTableIPv4Static>
    <routeTableIPv4StaticId>1</routeTableIPv4StaticId>
    <Dst>
    """ + build_netconf_attribute("dstId", dst_id) + """
    """ + build_netconf_attribute("dst", dst) + """
    </Dst>
    </RouteTableIPv4Static>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_nexthop(me_id, router_id, dst_id, nexthop_id, address, admin_distance):
 
    nexthop_value=""
         
    if address == "discard":
        nexthop_value = "<discard/>"
    else:
        nexthop_value = build_netconf_attribute("address", address)

    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <RouteTableIPv4Static>
    <routeTableIPv4StaticId>1</routeTableIPv4StaticId>
    <Dst>
    """ + build_netconf_attribute("dstId", dst_id) + """
    <NextHop>
    """ + build_netconf_attribute("nextHopId", nexthop_id) + """
    """ + nexthop_value + """
    """ + build_netconf_attribute("adminDistance", admin_distance) + """    
    </NextHop>
    </Dst>
    </RouteTableIPv4Static>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_vrrp_interface (me_id, vrrp_if_id, preempt_mode, preempt_holdtime, advertise_interval, vr_id, user_label = None):
    
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Vrrpv3Interface>
    """ + build_netconf_attribute("vrrpv3InterfaceId", vrrp_if_id) + """
    """ + build_netconf_attribute("preemptMode", preempt_mode) + """
    """ + build_netconf_attribute("preemptHoldTime", preempt_holdtime) + """
    """ + build_netconf_attribute("advertiseInterval", advertise_interval) + """
    """ + build_netconf_attribute("vrIdentity", vr_id) + """
    """ + build_netconf_attribute("userLabel", user_label) + """
    </Vrrpv3Interface>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config   
def create_vrrp_address (me_id, vrrp_if_id, address_id, address):
    
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Vrrpv3Interface>
    """ + build_netconf_attribute("vrrpv3InterfaceId", vrrp_if_id) + """
    <AddressIPv4>
    """ + build_netconf_attribute("addressIPv4Id", address_id) + """
    """ + build_netconf_attribute("address", address) + """
    </AddressIPv4>
    </Vrrpv3Interface>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_vrrp_session(me_id, router_id, interface_id, vrrp_sess_id, vrrp_if_name, prioritized_session, user_label = None):
    out = """<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <InterfaceIPv4>
    """ + build_netconf_attribute("interfaceIPv4Id", interface_id) + """
    <Vrrpv3Session>
    """ + build_netconf_attribute("vrrpv3SessionId", vrrp_sess_id) + """
    """ + build_netconf_attribute("vrrpv3InterfaceRef", build_vrrp_interface_dn(me_id, vrrp_if_name = vrrp_if_name)) + """
    """ + build_netconf_attribute("prioritizedSession", prioritized_session) + """
    """ + build_netconf_attribute("userLabel", user_label) + """
    </Vrrpv3Session>
    </InterfaceIPv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""
    return out

@execute_netconf_edit_config
def create_acl(me_id, router_id, acl_id, user_label = None):
    out ="""<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <AclIpv4>
    """ + build_netconf_attribute("aclIpv4Id", acl_id) + """
    """ + build_netconf_attribute("userLabel", user_label) + """
    </AclIpv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""    
    return out

@execute_netconf_edit_config
def create_acl_entry(me_id, router_id, acl_id, acl_entry):
    out ="""<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <edit-config>
    <config xmlns:xc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">
    <ManagedElement>
    """ + build_netconf_attribute("managedElementId", me_id) + """
    <SystemFunctions>
    <systemFunctionsId>1</systemFunctionsId>
    <AxeFunctions>
    <axeFunctionsId>1</axeFunctionsId>
    <SystemComponentHandling>
    <systemComponentHandlingId>1</systemComponentHandlingId>
    <TransportM>
    <transportMId>1</transportMId>
    <Router>
    """ + build_netconf_attribute("routerId", router_id) + """
    <AclIpv4>
    """ + build_netconf_attribute("aclIpv4Id", acl_id) + """
    <AclEntryIpv4>
    """ + build_netconf_attribute("aclEntryIpv4Id", acl_entry.get_attribute("name")) + """
    """ + build_netconf_attribute("action", acl_entry.get_attribute("action")) + """
    """ + build_netconf_attribute("priority", acl_entry.get_attribute("priority")) + """
    """ + build_netconf_attribute("addrDst", acl_entry.get_attribute("dst_addr")) + """
    """ + build_netconf_attribute("addrSrc", acl_entry.get_attribute("src_addr")) + """
    """ + build_netconf_attribute("dscp", acl_entry.get_attribute("dscp")) + """
    """ + build_netconf_attribute("fragmentType", acl_entry.get_attribute("fragment_type")) + """
    """ + build_netconf_attribute("icmpType", acl_entry.get_attribute("icmp_type")) + """
    """ + build_netconf_attribute("ipProtocol", acl_entry.get_attribute("ip_protocol")) + """
    """ + build_netconf_attribute("portDst", acl_entry.get_attribute("dst_port")) + """
    """ + build_netconf_attribute("portSrc", acl_entry.get_attribute("src_port")) + """
    """ + build_netconf_attribute("userLabel", acl_entry.get_attribute("user_label")) + """
    </AclEntryIpv4>
    </AclIpv4>
    </Router>
    </TransportM>
    </SystemComponentHandling>
    </AxeFunctions>
    </SystemFunctions>
    </ManagedElement>
    </config>
    </edit-config>
    </rpc>
    ]]>]]>"""    
    return out

######## NETCONF session methods #################################################


#############################################################################################
#################################    MAIN    ################################################
#############################################################################################

doc = { PARAMETERS : {}, RESOURCES : {} }

sys.argv.pop(0)

# Parse all input files
load_yml(doc, flat_list(sys.argv))


# Load node specific parameters
if REMOTE_SESSION:
    HOST = get_parameter_value ( doc, "HOST" )
    PORT = get_parameter_value ( doc, "PORT" )

    USERNAME = get_parameter_value ( doc, "USERNAME" )

APZ_MAG = get_parameter_value ( doc, "APZ_MAG" )
APT_MAG = get_parameter_value ( doc, "APT_MAG" )

# Read ME ID
me_id = get_me_id()

if me_id == None:
    ### FOR TEST
    me_id = 'TestMeId'

# Load resources in separate list to create the object in proper order
vlan_list=[]
subnetvlan_list=[]
router_list=[]
bfd_profile_list=[]
if_list=[]
static_route_list=[]
vrrp_if_list = []
port_list = []
acl_list = []

for item in doc[RESOURCES]:
    res = Resource(doc, doc[RESOURCES], item)
    elem_type = res.get_type()
    if elem_type == VLAN_TYPE:
        vlan_list += [res]
    elif elem_type == SUBNETVLAN_TYPE:
        subnetvlan_list += [res]
    elif elem_type == VRRP_INTERFACE_TYPE:
        vrrp_if_list += [res]
    elif elem_type == ROUTER_TYPE:
        router_list += [res]
    elif elem_type == BFD_PROFILE_TYPE:
        bfd_profile_list += [res]
    elif elem_type == INTERFACE_TYPE:
        if_list += [res]
    elif elem_type == STATIC_ROUTE_TYPE:
        static_route_list += [res]
    elif elem_type == PORT_TYPE:
        port_list += [res]
    elif elem_type == ACL_TYPE:
        acl_list += [res]
       
                

# Create all VLANs
for vlan in vlan_list:
    vlan.print_object()
    for mag, slot, plane in itertools.product(vlan.get_mag_address(), vlan.get_slot(), vlan.get_plane()):
        bridge_id = build_bridge_id(mag, slot, plane)
        
        sys.stdout.write("Creating Vlan %s on %s %s %s... " % (vlan.get_attribute("name"), mag, slot, plane) )
        create_vlan(me_id, 
                    bridge_id, 
                    vlan.get_attribute("name"), 
                    vlan.get_attribute("vid"), 
                    build_port_dn(me_id, bridge_id, port_id = vlan.get_attribute("tagged_ports")), 
                    build_port_dn(me_id, bridge_id, port_id = vlan.get_attribute("untagged_ports")),
                    user_label = vlan.get_attribute("user_label"))
        sys.stdout.write("\n")
        
           
# Create all Subnet Based VLANs
for subnetvlan in subnetvlan_list:
    subnetvlan.print_object()
    for mag, slot, plane in itertools.product(subnetvlan.get_mag_address(), subnetvlan.get_slot(), subnetvlan.get_plane()):
        bridge_id = build_bridge_id(mag, slot, plane)

        #Get the list of subnet VLANs already defined first and then add the new entry (if needed)
        subnet_vlan_attr = ""
        for existing_subnetvlan in get_subnet_vlan(me_id, bridge_id):
            
            if existing_subnetvlan.get_attribute("ipv4subnet") != subnetvlan.get_attribute("ipv4subnet") and existing_subnetvlan.get_attribute("ipv4subnet") != None:
                subnet_vlan_attr += str(build_subnet_vlan_struct(existing_subnetvlan.get_attribute("ipv4subnet"), existing_subnetvlan.get_attribute("vid")))
                
        subnet_vlan_attr += str(build_subnet_vlan_struct(subnetvlan.get_attribute("ipv4subnet"),subnetvlan.get_attribute("vid")))
    
        
        sys.stdout.write("Creating Subnet Vlan %s on %s %s %s... " % (subnetvlan.get_attribute("ipv4subnet"), mag, slot, plane) )
        create_subnetvlan(me_id, bridge_id, subnet_vlan_attr)
        sys.stdout.write("\n")

# Create all VRRP Interfaces
for vrrp_if in vrrp_if_list:
  
    vrrp_if.print_object()
    
    existing_vrrp_if = get_vrrp_interface(me_id, vrrp_if.get_attribute("name"))
    
    if len(existing_vrrp_if) > 0:
        sys.stdout.write("Modifying VRRP Interface... ")
        vr_id = None
    else:
        sys.stdout.write("Creating VRRP Interface... ")
        vr_id = vrrp_if.get_attribute("vr_id")
    
    create_vrrp_interface(me_id,
                          vrrp_if.get_attribute("name"), 
                          vrrp_if.get_attribute("preempt_mode"), 
                          vrrp_if.get_attribute("preempt_holdtime"), 
                          vrrp_if.get_attribute("advertise_interval"),
                          vr_id,
                          user_label = vrrp_if.get_attribute("user_label"))
        
    existing_vrrp_address = get_vrrp_address(me_id, vrrp_if.get_attribute("name"))
    #print existing_vrrp_address
    if len(existing_vrrp_address) > 0:
        if existing_vrrp_address[0].get_attribute("address") != vrrp_if.get_attribute("address"):
                sys.stdout.write("Modifying VRRP Address... ")
                create_vrrp_address(me_id,
                                    vrrp_if.get_attribute("name"),
                                    existing_vrrp_address.get_attribute("name"),
                                    vrrp_if.get_attribute("address"))
        else:
            sys.stdout.write("VRRP Address already set.\n")
    else:
        sys.stdout.write("Creating VRRP Address... ")
        create_vrrp_address(me_id, 
                            vrrp_if.get_attribute("name"),
                            vrrp_if.get_attribute("name"),
                            vrrp_if.get_attribute("address"))
    sys.stdout.write("\n")
    
# Create all Routers
for router in router_list:
    router.print_object()
    for mag, slot in itertools.product(router.get_mag_address(), router.get_slot()): 
        router_id = build_router_id(mag, slot, router.get_attribute("name"))
        sys.stdout.write("Creating Router %s on %s %s... " % (router.get_attribute("name"), mag, slot))
        create_router(me_id, router_id, user_label = router.get_attribute("user_label"))        
        sys.stdout.write("\n")

# Create all BFD Profiles
for profile in bfd_profile_list:
    profile.print_object()
    for mag, slot in itertools.product(profile.get_mag_address(), profile.get_slot()):
            
        router_id = build_router_id(mag, slot, profile.get_attribute("router_name"))
    
        sys.stdout.write("Creating BFD Profile %s on %s %s %s... " % (profile.get_attribute("name"), mag, slot, profile.get_attribute("router_name")))
        create_bfd_profile(me_id, 
                           router_id,
                           profile.get_attribute("name"),
                           profile.get_attribute("det_multiplier"),
                           profile.get_attribute("int_rx_min_required"),
                           profile.get_attribute("int_tx_min_desired"))
        sys.stdout.write("\n")
        
for acl in acl_list:
    acl.print_object()
    
    for mag, slot, router_name in itertools.product(acl.get_mag_address(), acl.get_slot(), flat_list(acl.get_attribute("router_name"))):
        router_id = build_router_id(mag, slot, router_name)
        
        sys.stdout.write("Creating ACL %s on %s %s %s... " % (acl.get_attribute("name"), mag, slot, router_name))
        create_acl(me_id, 
                   router_id, 
                   acl.get_attribute("name"), 
                   user_label = acl.get_attribute("user_label"))
        
        sys.stdout.write("\n")
        
        for entry_id in acl.get_attribute("acl_entries"):
            entry = Resource(doc, acl.get_attribute("acl_entries"), entry_id)
            entry.print_object()
            
            sys.stdout.write("Creating ACL entry %s:%s on %s %s %s... " % (acl.get_attribute("name"), entry.get_attribute("name"), mag, slot, router_name))
            create_acl_entry(me_id, 
                             router_id, 
                             acl.get_attribute("name"), 
                             entry)
            
            sys.stdout.write("\n")


# Create all Interfaces
for interface in if_list:
    interface.print_object()
    
    for mag, slot in itertools.product(interface.get_mag_address(), interface.get_slot()):
        router_id = build_router_id(mag, slot, interface.get_attribute("router_name"))
        
        bridge_id = build_bridge_id(mag, slot, TRANSPORT_POSTFIX)
        
        #Create interface or modify it if already exists
        existing_if = get_interface(me_id, router_id, interface.get_attribute("name"))  
        if len(existing_if) > 0 and existing_if[0].get_attribute("encapsulation_dn") == build_vlan_dn(me_id, bridge_id, vlan_name = interface.get_attribute("encapsulation")):
            sys.stdout.write("Modifying Interface %s on %s %s %s... " % (interface.get_attribute("name"), mag, slot, interface.get_attribute("router_name")) )
            encapsulation_dn = None
        else:
            sys.stdout.write("Creating Interface %s on %s %s %s... " % (interface.get_attribute("name"), mag, slot, interface.get_attribute("router_name")) ) 
            encapsulation_dn = build_vlan_dn(me_id, bridge_id, vlan_name = interface.get_attribute("encapsulation"))
        
        create_interface(me_id, 
                        router_id, 
                        interface.get_attribute("name"), 
                        interface.get_attribute("admin_state"),
                        encapsulation_dn, 
                        interface.get_attribute("mtu"),
                        interface.get_attribute("bfd_profile"),
                        build_acl_dn(me_id, router_id, acl_name = interface.get_attribute("acl_ingress")),
                        user_label = interface.get_attribute("user_label"))    
            
        
        #Create address if it doesn't exist
        existing_address = get_interface_address(me_id, router_id, interface.get_attribute("name"))
        if len(existing_address) > 0: 
            if existing_address[0].get_attribute("address") != interface.get_attribute("address"):
                
                sys.stdout.write("Modifying Address %s on %s %s %s... " % (interface.get_attribute("name"), mag, slot, interface.get_attribute("router_name")))
                create_address(me_id, 
                               router_id, 
                               interface.get_attribute("name"), 
                               existing_address[0].get_attribute("address"), 
                               interface.get_attribute("address"))
            else:
                sys.stdout.write("Address already set.\n")
        else:
            sys.stdout.write("Creating Address %s on %s %s %s... " % (interface.get_attribute("name"), mag, slot, interface.get_attribute("router_name")))
            create_address(me_id, 
                           router_id, 
                           interface.get_attribute("name"), 
                           interface.get_attribute("name"), 
                           interface.get_attribute("address"))
        
        
        #Create VRRP session if requested and it doesn't exist
        if (None != interface.get_attribute("vrrp_if_name")):
            existing_vrrp_sess = get_vrrp_session(me_id, router_id, interface.get_attribute("name"))
            
            if (len(existing_vrrp_sess) > 0 and  
            existing_vrrp_sess[0].get_attribute("vrrp_prioritized_session").upper() == interface.get_attribute("vrrp_prioritized_session").upper() and 
            existing_vrrp_sess[0].get_attribute("vrrp_if_name") == build_vrrp_interface_dn(me_id, interface.get_attribute("vrrp_if_name"))):
                sys.stdout.write("VRRP Session already set.\n")
            else: 
                sys.stdout.write("Creating VRRP Session %s on %s %s %s... " % (interface.get_attribute("vrrp_if_name"), mag, slot, interface.get_attribute("router_name")))
                create_vrrp_session(me_id, 
                                    router_id, 
                                    interface.get_attribute("name"), 
                                    interface.get_attribute("vrrp_if_name"), 
                                    interface.get_attribute("vrrp_if_name"), 
                                    interface.get_attribute("vrrp_prioritized_session").upper())
    
        sys.stdout.write("\n")
        
# Create all Static Routes
for static_route in static_route_list:
    static_route.print_object()
    
    for mag, slot in itertools.product(static_route.get_mag_address(), static_route.get_slot()):
        router_id = build_router_id(mag, slot, static_route.get_attribute("router_name"))
        
        dst_index = 0
        
        for route in flat_list(static_route.get_attribute("route")):
            for dst in route:
                dst_index += 1
                dst_name = str(dst_index)
                dst_network = dst
                nexthops = route[dst]
                
                existing_dst = get_dst(me_id, router_id, dst_network)
                
                if len(existing_dst) == 0:
                    sys.stdout.write("Creating Dst %s on %s %s %s... " % (dst_network, mag, slot, static_route.get_attribute("router_name")))
                    create_dst(me_id, 
                               router_id, 
                               dst_name, 
                               dst_network)
                else:
                    sys.stdout.write("Dst already set.\n")
                    dst_name = existing_dst[0].get_attribute("name")
            
                existing_nexthops = get_nexthop(me_id, router_id, dst_name)
            
                for nexthop_id in nexthops:
                    
                    admin_distance = nexthops[nexthop_id]
                    nexthop_address = nexthop_id
                    already_exist = False
                    
                    for item in existing_nexthops:                
                        if ("discard" == nexthop_address and item.get_attribute("discard") != None) or item.get_attribute("address") == nexthop_address:   
                            already_exist = True
                            msg = "Modifying NextHop %s on %s %s %s... " % (nexthop_id, mag, slot, static_route.get_attribute("router_name"))
                            sys.stdout.write(msg)
                            create_nexthop(me_id, 
                                           router_id, 
                                           dst_name, 
                                           item.get_attribute("name"),
                                           None, 
                                           admin_distance)
                            break
                        
                    if already_exist == False:    
                        msg = "Creating NextHop %s on %s %s %s... " % (nexthop_id, mag, slot, static_route.get_attribute("router_name"))
                        sys.stdout.write(msg)
                        create_nexthop(me_id, 
                                       router_id, 
                                       dst_name, 
                                       nexthop_address.replace('.','_'), 
                                       nexthop_address, 
                                       admin_distance)
                    
                sys.stdout.write("\n")
        
#Set adminState for specific ports
for port in port_list:
    port.print_object()
    for mag, slot, plane in itertools.product(port.get_mag_address(), port.get_slot(), port.get_plane()):
        bridge_id = build_bridge_id(mag, slot, plane)
        
        for port_name in flat_list(port.get_attribute("name")):
            msg = "Setting Port %s on %s %s %s... " % (port_name, mag, slot, plane)
            sys.stdout.write(msg)
            set_port(me_id,bridge_id, port_name, port.get_attribute("admin_state"), user_label = port.get_attribute("user_label"))
            sys.stdout.write("\n")
            
        
        