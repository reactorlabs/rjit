#!/usr/bin/env python3

import time
import os
import sys
import xml.etree.ElementTree as et

FORCE = False

def debug(aStr):
    # Change to debug
    if (False):
        print(aStr)


def error(file, line, message, fail=True):
    """ Displays an error with proper formatting and exits the application
    immediately, unless fail == False.
    """
    print("{file}:{line}: error: {message}".format(
        file=file, line=line, message=message), file=sys.stderr)
    if (fail):
        sys.exit(-1)


class Manager:

    """ Manages the doxygen generated xml classes for header files
    documentation.  """

    # TODO how to deal with underscores? __ perhaps - I need to check this
    @staticmethod
    def mangle(name):
        """ Mangles the given cpp name into doxygen filename format (:: == _1_1
        and capital leters == _lowercase).  """
        result = ""
        for c in name:
            if (c == ":"):
                result += "_1"
            elif (c.isupper()):
                result += "_" + c.lower()
            elif (c == "_"):
                result += "__"
            else:
                result += c
        return result

    @staticmethod
    def demangle(name):
        """ Reconstructs C++ style name from doxygen mangled one. """
        result = ""
        i = 0
        while (i < len(name)):
            if (name[i] == "_"):
                i += 1
                if (name[i] == "1"):
                    result += ":"
                else:
                    result += name[i].upper()
            else:
                result = result + name[i]
            i += 1
        return result

    def __init__(self, rootPath):
        """ Initializes the manager and sets the root path where the xml files
        are to be found.

        """
        self._rootPath = rootPath
        # dictitionary with already created classes
        self._classes = {}

    def _pathFor(self, name):
        """ Given a class name (including definition namespaces), returns the
        filename of that class' definition under the manager's root path.
        """
        return os.path.join(self._rootPath, "{0}.xml".format(name))

    def getClass(self, name):
        """ Loads the class with given name and all its children classes as
        well. If the class is already loaded, just returns it. """
        if (name in self._classes.keys()):
            return self._classes[name]
        fname = os.path.join(
            self._rootPath, "class{0}.xml".format(Manager.mangle(name)))
        if (os.path.isfile(fname)):
            c = CppClass(name, et.parse(fname), self)
            self._classes[name] = c
            c.load(self)
            return c
        assert(False)

    def getTypeFromXML(self, xml):
        if (xml.text):
            return CppUnknownType(xml.text)

        # or it is a known type in which case we load it through the manager
        else:
            if (len(xml) == 0):
                # it's a constructor or destructor, we are not interested in
                # these
                return None

            else:
                xml = xml[0]
                if (xml.tag != "ref"):
                    print("Expected ref section in method argument type")
                    sys.exit(-1)
                # now if the refid is actually a filename, then load the corresponding class
                # otherwise use it as type
                fname = self._pathFor(xml.attrib["refid"])
                if (not os.path.isfile(fname)):
                    return CppUnknownType(xml.text)
                
                klass = self.getClass(Manager.demangle(xml.attrib["refid"][5:]))  # [5:] to strip the leading class
                return CppClassType(klass, xml.tail)

class CppType(object):
    def isHandler(self):
        return self.name() == "handler"

    def isBool(self):
        return self.name() == "bool"

    def isSubclassOf(self, base):
        return False
    
    def name(self):
        raise NotImplementedError("")


class CppUnknownType(CppType):
    """ An unknown cpp type outside the doxygen documentation.
    """
    def __init__(self, name):
        self.name_ = name

    def name(self):
        return self.name_
    
    def __str__(self):
        return self.name()


class CppClassType(CppType):
    def __init__(self, klass, storageType):
        if storageType:
            self.storageType = storageType.replace(" ", "")
        else:
            self.storageType = ""
        self.klass = klass

    def isHandler(self):
        return False

    def isPointerType(self):
        return self.storageType == "*"

    def isSubclassOf(self, base):
        return self.klass.isSubclassOf(base)

    def name(self):
        return self.klass.name

    def matchSet(self):
        return self.klass.matchSet

    def methods(self):
        return self.klass.methods

    def __str__(self):
        return "{0} {1}".format(self.name(), self.storageType)

    def equals(self, other):
        return self.klass == other.klass and self.storageType == other.storageType

class CppClass:

    """ C++ class
    """

    def __init__(self, name, xml, manager):
        """ Creates the class with given xml.

        Does not yet load and analyze the data as this will be done in the
        second step to prevent circular definitions.  """
        self.name = name
        self._xml = xml.getroot()[0]  # compounddef
        if (self._xml.attrib["kind"] != "class"):
            print("Error: {0} is not a class".format(name))
            exit(-1)
        self.parents = []
        self.manager = manager
        # print("Class {0} initialized from xml".format(name))

    def addParent(self, c):
        """ Adds given class to the list of parents of the class. """
        self.parents.append(c)

    def load(self, manager):
        self.subclasses = []
        self.methods = []
        for child in self._xml:
            if (child.tag == "location"):
                self.file = child.get("file")
                self.line = child.get("line")
            elif (child.tag == "derivedcompoundref"):
                c = manager.getClass(child.text)
                self.subclasses.append(c)
                c.addParent(self)
            elif (child.tag == "sectiondef"):
                if (child.get("kind") in ("public-func", "private-func", "protected-func")):
                    for method in child:
                        self.methods.append(CppMethod(method, manager))

    def isSubclassOf(self, base):
        if (base == self):
            return True
        if (base in self.parents):
            return True
        for p in self.parents:
            if (p.isSubclassOf(base)):
                return True
        return False

    def isMatcher(self):
        return self.isSubclassOf(
                self.manager.getClass("rjit::ir::Matcher"))


class CppMethod:

    """ C++ method
    """

    def __init__(self, xml, manager):
        """ Initializes the method from the given xml. """
        self._xml = xml
        self.args = []
        self.overrides = False
        for child in xml:
            if (child.tag == "type"):
                self.type = manager.getTypeFromXML(child)
            elif (child.tag == "name"):
                self.name = child.text
            elif (child.tag == "location"):
                self.file = child.get("file")
                self.line = child.get("line")
            elif (child.tag == "param"):
                self.args.append(CppVariable(child, manager))
            elif (child.tag == "reimplements"):
                self.overrides = True

        self.manager = manager

    def __str__(self):
        return "{0} {1}".format(self.type, self.name)

    def isHandler(self):
        return self.type and self.type.isHandler()

    def checkHandler(self):
        """ Checks that the method as a handler is fine. This means to 1) check
        that its instructions come before predicates and that there are no
        other arguments. It also checks that all predicates conform to the
        handler's signature.
        """
        # at least one argument
        if (len(self.args) == 0):
            error(self.file, self.line,
                  "Handler method {0} is not allowed to take no arguments.".format(self))
        # all args but last must be instructions
        for a in self.args[0:-1]:
            if not a.isMatcher():
                error(self.file, self.line, "Handler method {0}, argument {1}: handler's non-last arguments must be instructions matchers, i.e. inherit from ir::Instruction".format(self, a))
        # last argument can either be instruction or predicate
        if (len(self.args) > 1):
            a = self.args[-1]
            if (a.isMatcher()):
                return  # all good
            elif a.isPredicate():
                p = a.type
                # find predicate has static method named match
                for m in p.methods():
                    if (m.name == "match"):
                        if (m.isStatic()):
                            error(
                                m.file, m.line, "Predicate '{0}' match method '{1}' must not be static".format(p, m))
                        if (not m.type.isBool()):
                            error(
                                m.file, m.line, "Predicate '{0}' match method '{1}' must return bool.".format(p, m))
                        if (len(m.args) != len(self.args)):
                            error(
                                m.file, m.line, "Predicate '{0}' match method '{1}' does not have proper signature - invalid number of arguments.".format(p, m), fail=False)
                            error(
                                self.file, self.line, "when used at handler {0}".format(self))
                        # first argument must be handler
                        if (not m.args[0].isHandler()):
                            error(
                                m.file, m.line, "Predicate {0} match method's first argument must be a rjit::ir::Handler &.".format(p))
                        for i in range(1, len(m.args)):
                            if (not m.args[i].type.equals(self.args[i - 1].type)):
                                error(
                                    m.file, m.line, "Predicate '{0}' match method's signature must follow the handler, difference at argument '{1}'".format(p, m.args[i]), fail=False)
                                error(
                                    self.file, self.line, "when used at handler {0}.".format(self))
                        return  # all good

                error(
                    p.file, p.line, "Predicate {0} does not define public method match.".format(p))

                pass  # check the predicate's signature to follow
            else:
                error(
                    self.file, self.line, "Handler method {0}, argument {1}: handler last arguments must inherit from either rjit::ir::Instruction or rjit::ir::Predicate.".format(self, a))

    def isVirtual(self):
        """ Returns true if the method is virtual. """
        return self._xml.attrib["virt"] == "virtual"

    def isStatic(self):
        """ Returns true if the method is static. """
        return self._xml.attrib["static"] != "no"

    def matchSequence(self):
        """ Returns a list of Instruction types that the method, assuming it is a handler matches. """
        result = []
        for a in self.args:
            if not a.isMatcher():
                if not a.isPredicate():
                    print("Argument '{0}' to '{1}' is not a valid matcher".format(a, self))
                    sys.exit(1)
            else:
                result.append(a.type)
        return result

    def predicate(self):
        """ Returns the predicate associated with the handler, or False if the handler is unconditional. """
        for a in self.args:
            if a.isPredicate():
                return a.type
        return False


class CppVariable:

    """ C++ variable
    """

    def __init__(self, xml, manager):
        self._xml = xml
        self.text = ""
        self.name = ""
        for child in xml:
            if (child.tag == "declname"):
                self.name = child.text
            elif (child.tag == "type"):
                self.type = manager.getTypeFromXML(child)
        self.manager = manager

    def __str__(self):
        return "{0} {1}".format(self.type, self.name)

    def isMatcher(self):
        return self.type.isSubclassOf(
                self.manager.getClass("rjit::ir::Matcher")) and \
                        self.type.isPointerType()

    def isPredicate(self):
        return self.type.isSubclassOf(
                self.manager.getClass("rjit::ir::Predicate"))
    
    def isHandler(self):
        return self.type.isSubclassOf(
                self.manager.getClass("rjit::ir::Handler"))



class Handler:

    """ Handler information.

    Each child of ir::Handler that has at least one non overriding handler must
    have its dispatch method overriden. This class keeps record of each such
    class.
    """
    class DispatchTable:

        class Entry:

            def __init__(self, type):
                self.type = type
                self.conditional = []
                self.unconditional = False
                self.unconditionalMatchLength = False
                self.recursive = Handler.DispatchTable()

            def __str__(self):
                result = "{0}: {1} or ".format(
                    self.type, self.unconditional.name if self.unconditional else "nodefault")
                for m in self.conditional:
                    result += m.name + " "
                if (self.recursive):
                    result += "\n    " + \
                        self.recursive.__repr__().replace("\n", "\n    ")
                return result

            def _addHandlerMethod(self, handlerMethod, matchSequence):
                if (len(matchSequence) == 1):
                    if (handlerMethod.predicate()):
                        self.conditional.append(handlerMethod)
                    else:
                        mss = len(matchSequence[0].matchSet())
                        if (not self.unconditionalMatchLength or mss < self.unconditionalMatchLength):
                            self.unconditionalMatchLength = mss
                            self.unconditional = handlerMethod
                        elif (mss == self.unconditionalMatchLength):
                            error(handlerMethod.file, handlerMethod.line,
                                  "Ambiguous handler for instruction type {0}".format(self.type), fail=False)
                            error(
                                self.unconditional.file, self.unconditional.line,
                                  "Previous match in handler method {0}".format(self.unconditional.name))
                else:
                    self.recursive._addHandlerMethod(
                        handlerMethod, matchSequence[1:])

            def emitUnconditionalCall(self, iterators):
                result = "("
                for i in range(0, len(iterators)-1):
                    result += "static_cast<{0}>(".format(self.unconditional.matchSequence()[i])
                    result += "Instruction::getIR("+iterators[i]+"))"
                    if i < len(iterators) - 2:
                        result += ","
                result += ")"
                return result

            def emitPredicateCall(self, conditional, iterators):
                result = "(*this, "
                for i in range(1, len(iterators)):
                    result += "static_cast<{0}>(".format(conditional.matchSequence()[i-1])
                    result += "Instruction::getIR("+iterators[i]+"))"
                    if i < len(iterators) - 1:
                        result += ","
                result += ")"
                return result

            def emitUnconditional(self, iterators):
                if (self.unconditional):
                    result = self.unconditional.name + \
                        self.emitUnconditionalCall(iterators)
                    return result

            def emitConditional(self, iterators, conditional):
                predicate = conditional.predicate()
                return """{{
        {ptype} p;
        if (p.match{sigP}) {{
            {hname}{sig};
            goto DONE;
        }}
    }}""".format(ptype=predicate.name(), sigP=self.emitPredicateCall(conditional, iterators), sig=self.emitUnconditionalCall(iterators), hname=conditional.name)

            def emit(self, iterators):
                """ Emits C++ code for the dispatch table entry.

                The incoming iterators contains all iterators that were used in the dispatch so far,
                """
                result = "case Instruction::InstructionKind::{0}: {{\n".format(self.type)
                if (self.recursive):
                    result += """    if (not {matched}->isTerminator()) {{
        {dispatch}
    }}
""".format(matched=iterators[-2], dispatch=self.recursive.emit(iterators[:]).replace("\n", "\n        "))

                # first check the conditional ones
                for h in self.conditional:
                    result += "    {0}\n".format(
                        self.emitConditional(iterators, h))

                # then go for the single non-conditional one
                if (self.unconditional):
                    result += "    {0};\n".format(
                        self.emitUnconditional(iterators))
                    # assign the last iterator to the first one (first one is
                    # ref arg)
                    result += "    {first} = {last};\n    return true;\n".format(
                        first=iterators[0], last=iterators[-1])

                result += "}\n"
                return result
                pass

        def __init__(self):
            self._table = {}

        def __bool__(self):
            return True if (self._table) else False

        def __str__(self):
            result = "\n".join([x.__str__() for x in self._table.values()])
            return result

        def __repr__(self):
            return self.__str__()

        def _getOrCreateEntry(self, type):
            if (type in self._table.keys()):
                return self._table[type]
            x = Handler.DispatchTable.Entry(type)
            self._table[type] = x
            return x

        def _addHandlerMethod(self, handlerMethod, matchSequence):
            """ Adds given handler method and all it matches into the dispatch
            table. Takes the match signature of the handler method as well as
            an index to the signature - this is for recursive matching to
            determine how deep in the recursion we are. """
            handlerMethod.checkHandler()
            ir = matchSequence[0]
            for m in ir.matchSet():
                self._getOrCreateEntry(m)._addHandlerMethod(
                    handlerMethod, matchSequence)

        def emit(self, incomingIterators):
            """ Emits the C++ code for the given dispatch table, taking the
            name of the incomming iterator storing the current instruction for
            sequential matching. The outgoing iterator is always i as per the
            dispatch function signature.
            """
            # last iterator is the iterator on which we should call the match
            # function to get the type of instruction to dispatch on
            lastIterator = incomingIterators[-1]
            it = lastIterator + lastIterator[0]
            # add the last iterator to incoming iterators so that entry's emits
            # will find it
            incomingIterators.append(it)
            result = """llvm::BasicBlock::iterator {it} = {incoming};
if (!rjit::ir::Instruction::isInstruction({it})) return false;
Instruction* instruction = rjit::ir::Instruction::match({it});
switch (instruction->getKind()) {{
""".format(it=it, incoming=lastIterator)
            for entry in self._table.values():
                result += entry.emit(incomingIterators)
            result += "}"
            return result

    def __init__(self, handlerClass):
        """ Initializes the handler from given class. It is assumed that the
        class is a ir::Handler child. Fills in the handler's list.  """
        self.handlerClass = handlerClass
        self.handlerMethods = []
        for m in handlerClass.methods:
            if (m.isHandler() and not m.overrides):
                self.handlerMethods.append(m)

    def hasHandlers(self):
        return len(self.handlerMethods) > 0

    def buildDispatchTable(self):
        self._table = Handler.DispatchTable()
        for h in self.handlerMethods:
            self._table._addHandlerMethod(h, h.matchSequence())

    def destFile(self, dest):
        """ Returns the target file for the codegen. This consists of the name
        of the handler class wih :: replaced by _ in the given destination
        directory. """
        return os.path.join(dest, self.handlerClass.name.replace("::", "_") + ".cpp")

    def shouldEmit(self, dest):
        global FORCE
        """ Returns true if the particular handler should be created. This
        happens if either the source of the handler's class file is newer than
        the handler's autogenerated code, or if the handler's autogenerated
        code cannot be found at all.
        """
        if (FORCE):
            return True
        if (not os.path.isfile(self.handlerClass.file)):
            print(
                "Unable to locate source file {0}".format(self.handlerClass.file))
            return True
        ts = os.path.getmtime(self.handlerClass.file)
        df = self.destFile(dest)
        if (not os.path.isfile(df)):
            return True
        td = os.path.getmtime(df)
        return ts > td

    def emit(self, dest, cppBase):
        parents = ""
        header = self.handlerClass.file[len(cppBase) + 1:]
        for p in self.handlerClass.parents:
            parents += "if ({0}::dispatch(i))\n            goto DONE;\n".format(p.name)
        code = """#include "{header}"
#include "llvm.h"
#include "RIntlns.h"
#include "ir/Intrinsics.h"

#pragma GCC diagnostic ignored "-Wswitch"
bool {handler}::dispatch(llvm::BasicBlock::iterator & i) {{
    bool success = true;

    {code}
    {parents}

    success = false;
DONE:
    i = ii;
    return success;
}}""".format(header=header, handler=self.handlerClass.name, code=self._table.emit(["i"]).replace("\n", "\n    "), parents=parents)

        content = ""
        if os.path.isfile(self.destFile(dest)):
            with open(self.destFile(dest), 'r') as f:
                content = f.read()
        if not content == code:
            print(self.destFile(dest), "updated")
            with open(self.destFile(dest), "w") as f:
                f.write(code)


def analyzeMatchSets(klass):
    """ Analyzes the match sets of all subclasses of the given class.

    A set of string names XXX for the ir::Type::XXX will be added to each
    subclass of rjit::ir::Instruction as matchSet attribute.

    It is expected that main program only calls this function with the
    rjit::ir::Instruction as argument.
    """

    # if the match set has already been calculated, return it
    if (hasattr(klass, "matchSet")):
        return klass.matchSet

    # calculate the match set
    m = set()
    if (not hasattr(klass, 'subclasses')):
        print("cannot extract subclasses: probably doxygen command failed to generate output")
        sys.exit(-1)

    subclasses = set(klass.subclasses)

    # if the class is a leaf in the hierarchy, its matchset is its own name
    if not subclasses and klass.isMatcher():
        m.add(klass.name.split("::")[-1])
        klass.matchSet = m
        return m

    # otherwise the matchset of the class is union of all match sets of its
    # children
    for child in subclasses:
        m = m.union(analyzeMatchSets(child))

    klass.matchSet = m
    return m


def analyzeHandlers(c, dest):
    """ Analyzes the handler classes and extracts their handler methods. """
    handlers = []

    h = Handler(c)
    if (h.hasHandlers()):
        # check that we should emit the handler
        if (h.shouldEmit(dest)):
            handlers.append(h)

    for child in c.subclasses:
        handlers += analyzeHandlers(child, dest)

    return handlers


def usage(err=""):
    print("usage: " + str(sys.argv[0]) + " doxygen_out_dir target_dir")
    print(err)
    sys.exit(-1)


def main():
    global FORCE

    # we take two arguments - where to look for the doxygen xmls and where to
    # put the codegens
    if (len(sys.argv) < 3):
        usage()
    
    cppBase = os.path.abspath(str(sys.argv[0]))
    cppBase = cppBase[:cppBase.find("/tools/codegen_handlers.py")]
    cppBase = os.path.join(cppBase, "rjit", "src")
    sources = str(sys.argv[1])
    dest = str(sys.argv[2])
    if (len(sys.argv) == 4 and sys.argv[3] == "force"):
        FORCE = True
    
    if (not os.path.isdir(sources)):
        usage("Sources directory does not exist")
    
    if (not os.path.isdir(dest)):
        usage("Dest directory does not exist")
    

    lastUpdate = os.path.getmtime(dest)
    codeGenChanged = os.path.getmtime(os.path.realpath(__file__))
    if lastUpdate < codeGenChanged:
        FORCE = True

    # now we know we have both source and dest dirs. initialize the sources manager
    debug("initializing...")
    m = Manager(sources)
    # load the classes we required
    debug("loading...")
    debug("    instructions")
    ir_ins = m.getClass("rjit::ir::Matcher")
    debug("    handlers")
    ir_handler = m.getClass("rjit::ir::Handler")
    
    # analyze the data - create match sets and handlers
    debug("analyzing...")
    debug("    instruction match sets...")
    analyzeMatchSets(ir_ins)
    debug("    handlers...")
    handlers = analyzeHandlers(ir_handler, dest)
    debug("    handler dispatch tables...")
    # create handler dispatch tables
    for h in handlers:
        h.buildDispatchTable()
        if (h.handlerClass != ir_handler):
            h.emit(dest, cppBase)
    # update mtime of the folder
    now = time.time()
    os.utime(dest, (now, now))

if __name__ == "__main__":
   # main()
   pass
