/**
 * @file libcomp/src/Object.h
 * @ingroup libcomp
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Base class for an object generated by the object generator (objgen).
 *
 * This file is part of the COMP_hack Library (libcomp).
 *
 * Copyright (C) 2012-2016 COMP_hack Team <compomega@tutanota.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBCOMP_SRC_OBJECT_H
#define LIBCOMP_SRC_OBJECT_H

// Standard C++11 Includes
#include <stdint.h>
#include <functional>
#include <istream>
#include <list>
#include <memory>
#include <ostream>
#include <unordered_map>

// tinyxml2 Includes
#include <PushIgnore.h>
#include <tinyxml2.h>
#include <PopIgnore.h>

namespace libcomp
{

class Packet;
class ReadOnlyPacket;

/**
 * A data input stream wrapper with collection type size information
 * broken out into a seperate field.
 */
class ObjectInStream
{
public:
    /**
     * Create a stream and an empty dynamic size list.
     */
    ObjectInStream(std::istream& _stream) : stream(_stream) { }

    /// Input data stream
    std::istream& stream;

    /// List of dynamic sizes for collection types
    std::list<uint16_t> dynamicSizes;
};

/**
 * A data output stream wrapper with collection type size information
 * broken out into a seperate field.
 */
class ObjectOutStream
{
public:
    /**
     * Create a stream and an empty dynamic size list.
     */
    ObjectOutStream(std::ostream& _stream) : stream(_stream) { }

    /// Output data stream
    std::ostream& stream;

    /// List of dynamic sizes for collection types
    std::list<uint16_t> dynamicSizes;
};

/**
 * Abstract base class that represents any object that can be defined
 * via a MetaObject definition and corresonding value assignment
 * that can be read via XML.  Data can be save and loaded via byte
 * streams and also added to and retrieved from packets.
 * @sa DynamicObject
 * @sa PersistentObject
 */
class Object
{
public:
    /**
     * Create an object.
     */
    Object();

    /**
     * Clean up the object.
     */
    virtual ~Object();

    /**
     * Check if the object is currently in a valid state.
     * @param recursive If references to other objects exist
     *  and recursive = true, IsValid will be run on those
     *  references as well
     * @return true if valid, false if invalid
     */
    virtual bool IsValid(bool recursive = true) const = 0;

    /**
     * Load the object's data members from an ObjectInStream.
     * @param stream Byte stream containing data member values
     * @return true if loading was successful, false if it was not
     */
    virtual bool Load(ObjectInStream& stream) = 0;

    /**
     * Save the object's data members to an ObjectOutStream.
     * @param stream Byte stream to save data member values to
     * @return true if saving was successful, false if it was not
     */
    virtual bool Save(ObjectOutStream& stream) const  = 0;

    /**
     * Load the object's data members from a standard input stream.
     * @param stream Byte stream containing data member values
     * @param flat If references to non-persistent objects exist
     *  and flat = false those references' data members are specified
     *  in the stream as well
     * @return true if loading was successful, false if it was not
     */
    virtual bool Load(std::istream& stream, bool flat = false) = 0;

    /**
     * Save the object's data members to a standard output stream.
     * @param stream Byte stream to save data member values to
     * @param flat If references to non-persistent objects exist
     *  and flat = false those references' data members will be saved
     *  in the stream as well
     * @return true if saving was successful, false if it was not
     */
    virtual bool Save(std::ostream& stream, bool flat = false) const  = 0;

    /**
     * Load the object's data members from an XML file.
     * @param doc XML document containing the definition
     * @param root Root XML node of the definition
     * @return true if loading was successful, false if it was not
     */
    virtual bool Load(const tinyxml2::XMLDocument& doc,
        const tinyxml2::XMLElement& root) = 0;

    /**
     * Save the object's data members to an XML file.
     * @param doc XML document to save the definition to
     * @param root Root XML node to save the definition to
     * @return true if saving was successful, false if it was not
     */
    virtual bool Save(tinyxml2::XMLDocument& doc,
        tinyxml2::XMLElement& root) const = 0;

    /**
     * Load the object's data members from a data packet.
     * @param p Packet containing data member values
     * @param flat Specifies if the call to Load should be flat
     * @return true if loading was successful, false if it was not
     */
    virtual bool LoadPacket(libcomp::ReadOnlyPacket& p, bool flat = true);

    /**
     * Save the object's data members to a data packet.
     * @param p Packet to save data member values to
     * @param flat Specifies if the call to Save should be flat
     * @return true if saving was successful, false if it was not
     */
    virtual bool SavePacket(libcomp::Packet& p, bool flat = true) const;

    /**
     * Get the dynamic size count of the MetaObject definition.
     * An object's dynamic size count can be thought of as the
     * minimum size required to save an object of a given type.
     * @return MetaObject definition's dynamic size count
     */
    virtual uint16_t GetDynamicSizeCount() const = 0;

    /**
     * Static utility function to build multiple objects from
     * an input stream and a factory function.
     * @param stream Byte stream containing the object data
     * @param objectAllocator Factory function to build the objects
     * @return List of resulting objects built from the stream
     */
    static std::list<std::shared_ptr<Object>> LoadBinaryData(
        std::istream& stream, const std::function<
        std::shared_ptr<Object>()>& objectAllocator);

    /**
    * Static utliity function to save multiple objects to an output stream.
    * @param stream Byte stream to save the object data to.
    * @param objs List of objects to save to the stream.
    * @return true if saving was successful, false if it was not
    */
    static bool SaveBinaryData(std::ostream& stream,
        const std::list<std::shared_ptr<Object>>& objs);

protected:
    /**
     * Utility function to get a child XML element from a parent node
     * matching a specified name.
     * @param root Parent XML node
     * @param name Child element name to retrieve
     * @return First matching XML node child or nullptr if not found
     * @sa Object::GetXmlChildren
     */
    const tinyxml2::XMLElement* GetXmlChild(const tinyxml2::XMLElement& root, const std::string name) const;

    /**
     * Utility function to get child XML elements from a parent node
     * matching a specified name.
     * @param root Parent XML node
     * @param name Child element name to retrieve
     * @return List of matching XML node children or empty if not found
     * @sa Object::GetXmlChild
     */
    const std::list<const tinyxml2::XMLElement*> GetXmlChildren(const tinyxml2::XMLElement& root, const std::string name) const;

    /**
     * Utility function to get child XML elements from a parent node.
     * @param root Parent XML node
     * @return Map of child members by element name
     */
    virtual std::unordered_map<std::string, const tinyxml2::XMLElement*>
        GetXmlMembers(const tinyxml2::XMLElement& root) const;

    /**
     * Utility function to get element text from an XML node.
     * @param root Parent XML node
     * @return Text contained in the node's element text
     */
    virtual std::string GetXmlText(const tinyxml2::XMLElement& root) const;
};

} // namespace libcomp

#endif // LIBCOMP_SRC_OBJECT_H
