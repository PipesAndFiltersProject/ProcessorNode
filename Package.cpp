//
//  Package.cpp
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 28.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//


#include <boost/algorithm/string.hpp>
#include <vector>

#include <boost/uuid/uuid_io.hpp>

#include <OHARBaseLayer/Package.h>
#include <OHARBaseLayer/DataItem.h>


namespace OHARBase {
    
    
    const std::string Package::separatorStr("\\");
    const std::string Package::controlStr = "control";
    const std::string Package::dataStr = "data";
    const std::string Package::noTypeStr = "";
    
    /** Default constructor for Package. Generates a random uuid for the Package. */
    Package::Package()
    : uid(boost::uuids::random_generator()()), type(Package::Type::NoType), dataItem(nullptr)
    {
    }
    
    /** Copy constructor for Package. Copies the passed object.
     @param p The package to copy from. */
    Package::Package(const Package & p)
    : uid(p.uid), type(p.type), data(p.data), dataItem(nullptr)
    {
        setDataItem(p.getDataItem());
    }
    
    /** Move constructor for Package. Moves data from
     the passed object, transferring the ownership to this object.
     */
    Package::Package(Package && p)
    : uid(std::move(p.uid)), type(std::move(p.type)), data(std::move(p.data))
    {
        dataItem = std::move(p.dataItem);
        p.dataItem = nullptr;
    }
    
    /** A constructor giving an uuid for the otherwise empty package.
     @param id The uuid for the package. */
    Package::Package(const boost::uuids::uuid & id)
    : uid(id), type(Package::Type::NoType), dataItem(nullptr)
    {
    }
    
    /** A constructor giving type and data for the package. Uuid is generated.
     @param ptype The type for the package.
     @param d The data contents of the package. */
    Package::Package(Type ptype, const std::string & d)
    : uid(boost::uuids::random_generator()()), type(ptype), data(d), dataItem(nullptr)
    {
    }
    
    /** A constructor giving several parameters for the package.
     @param id The uuid for the package.
     @param ptype The type of the data (control or data package).
     @param d The data contents of the package. */
    Package::Package(const boost::uuids::uuid & id, Type ptype, const std::string & d)
    : uid(id), type(ptype), data(d), dataItem(nullptr)
    {
    }
    
    /** Destructor deletes the contained dataItem. */
    Package::~Package() {
        delete dataItem;
    }
    
    
    /** For querying the uuid of the package. */
    const boost::uuids::uuid & Package::getUuid() const {
        return uid;
    }
    
    /** For setting the uuid for the package.
     @param id The uuid for the package. */
    void Package::setUuid(const boost::uuids::uuid & id) {
        uid = id;
    }
    
    /** Can be used to check what kind of a data package is.
     @return The type of the data package. See type member variable for details. */
    Package::Type Package::getType() const {
        return type;
    }
    
    /** Sets the type of the package.
     @param t The type for this package. See the documentation of the
     member variable type for more information about the possible package types. */
    void Package::setType(Type t) {
        type = t;
    }
    
    /** Use the method to get a string representation of the type of the data
     item. Used in externalizing and internalizing Package objects to/from
     a stream (file, network).
     @return the Package type as string.*/
    const std::string & Package::getTypeAsString() const {
        switch (type) {
            case Control: {
                return controlStr;
            }
            case Data: {
                return dataStr;
            }
            default: {
                return noTypeStr;
            }
        }
    }
    
    /** Set the type of the package from a string. Useful when converting from JSON or text form to
     object.
     @param typeStr The type of the package as string.
     */
    void Package::setTypeFromString(const std::string & typeStr) {
        if (typeStr == Package::controlStr) {
            type = Package::Control;
        } else if (typeStr == Package::dataStr) {
            type = Package::Data;
        } else {
            type = Package::NoType;
        }
    }
    
    /** Get the unparsed data contents for the Package.
     @return the data of the package. */
    const std::string & Package::getData() const {
        return data;
    }
    
    /** Sets the unparsed data for the Package.
     @param d The data for this Package. */
    void Package::setData(const std::string & d) {
        data = d;
    }
    
    /** Use for getting the parsed, structured DataItem of
     the Package. May be null if there is no data or it has not
     been parsed from the data member variable.
     @return The pointer to the data item object. */
    const DataItem * Package::getDataItem() const {
        return dataItem;
    }
    
    /** Use for getting the modifiable pointer to the parsed,
     structured DataItem of the Package. May be null if there
     is no data or it has not been parsed from the data member variable.
     @return The pointer to the data item object. */
    DataItem * Package::getDataItem() {
        return dataItem;
    }
    
    /** Sets the new data item for this package, deleting the old one (if any).
     @param item The new dataitem for this package. DataItem is copied, so caller must handle
     the parameter object lifetime. */
    void Package::setDataItem(const DataItem * item) {
        if (dataItem) {
            delete dataItem;
            dataItem = nullptr;
        }
        if (item) {
            dataItem = item->copy();
        }
    }
    
    /** Use to query if package is empty. Package is empty if it has no type and dataItem is nullptr.
     @return Returns true if package is empty. */
    bool Package::isEmpty() const {
        return (type == NoType && dataItem == nullptr);
    }
    
    const Package & Package::operator = (const Package & p) {
        if (this != &p) {
            uid = p.uid;
            type = p.type;
            data = p.data;
            this->setDataItem(p.getDataItem());
        }
        return *this;
    }
    
    const Package & Package::operator = (Package && p) {
        if (this != &p) {
            uid = std::move(p.uid);
            type = std::move(p.type);
            data = std::move(p.data);
            dataItem = std::move(p.dataItem);
            p.setDataItem(0);
        }
        return *this;
    }
    
    
    bool Package::operator == (const Package & pkg) const {
        return (uid == pkg.uid);
    }
    
    bool Package::operator == (const std::string & str) const {
        return data == str;
    }
    
   
      /** Separator string for data transfer between nodes.
       @todo Remove separatorStr since it is not used after moving to JSON.
       */
    const std::string & Package::separator() {
        return separatorStr;
    }
    
    /** Externalizes the Package to a JSON object.
     @param j The JSON object fill with package contents.
     @param package The package to exernalize.
     */
    void to_json(nlohmann::json & j, const Package & package) {
        j = nlohmann::json{{"package", to_string(package.getUuid())}};
        j["type"] = package.getTypeAsString();
        j["payload"] = package.getData();
    }
    
    /** Internalizes the package contents from a JSON structure.
     @param j The JSON object containing Package data elements.
     @param package The package to initialize from the JSON structure.
     */
    void from_json(const nlohmann::json & j, Package & package) {
        if (j.find("package") != j.end()) {
            boost::uuids::string_generator gen;
            package.setUuid(gen(j["package"].get<std::string>()));
        }
        if (j.find("type") != j.end()) {
            package.setTypeFromString(j["type"].get<std::string>());
        }
        if (j.find("payload") != j.end()) {
            package.setData(j["payload"].get<std::string>());
        }
    }
    
    
    
} //namespace
