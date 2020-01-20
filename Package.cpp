//
//  Package.cpp
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 28.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <vector>


#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <g3log/g3log.hpp>

#include <ProcessorNode/Package.h>
#include <ProcessorNode/DataItem.h>


namespace OHARBase {
   
   const std::string Package::controlStr = "control";
   const std::string Package::dataStr = "data";
   const std::string Package::configurationStr = "configuration";
   const std::string Package::emptyString = "";
   
   /** Default constructor for Package. Generates a random uuid for the Package. */
   Package::Package()
   : uid(boost::uuids::random_generator()()), type(Package::Type::NoType), payload("")
   {
   }
   
   /** Copy constructor for Package. Copies the passed object.
    @param p The package to copy from. */
   Package::Package(const Package & p)
   : uid(p.uid), type(p.type), payload(""), originAddress(p.originAddress), destinationAddress(p.destinationAddress)
   {
      setPayloadVariant(p.payload);
   }
   
   /** Move constructor for Package. Moves data from
    the passed object, transferring the ownership to this object.
    */
   Package::Package(Package && p)
   :  uid(std::move(p.uid)), type(std::move(p.type)),
      originAddress(std::move(p.originAddress)), destinationAddress(std::move(p.destinationAddress))
   {
      payload = std::move(p.payload);
   }
   
   /** A constructor giving an uuid for the otherwise empty package.
    @param id The uuid for the package. */
   Package::Package(const boost::uuids::uuid & id)
   : uid(id), type(Package::Type::NoType), payload("")
   {
   }
   
   /** A constructor giving type and data for the package. Uuid is generated.
    @param ptype The type for the package.
    @param d The data contents of the package. */
   Package::Package(Type ptype, const std::string & d)
   : uid(boost::uuids::random_generator()()), type(ptype), payload(d)
   {
   }
   
   /** A constructor giving several parameters for the package.
    @param id The uuid for the package.
    @param ptype The type of the data (control or data package).
    @param d The data contents of the package. */
   Package::Package(const boost::uuids::uuid & id, Type ptype, const std::string & d)
   : uid(id), type(ptype), payload(d)
   {
   }
   
   /** Destructor. */
   Package::~Package() {
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
         case Configuration: {
            return configurationStr;
         }
         default: {
            return emptyString;
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
      } else if (typeStr == Package::configurationStr) {
         type = Package::Configuration;
      } else {
         type = Package::NoType;
      }
   }
   
   /** Get the unparsed data contents for the Package.
    @return the data of the package, empty string if parsed to Dataitem. */
   const std::string & Package::getPayloadString() const {
      auto item = std::get_if<std::string>(&payload);
      if (item) {
         return *item;
      }
      return emptyString;
   }
   
   /** Sets the unparsed data for the Package.
    @param d The data for this Package. */
   void Package::setPayload(const std::string & d) {
      payload = d;
   }
   
   /** Use for getting the parsed, structured DataItem of
    the Package. May be null if there is no data or it has not
    been parsed from the data member variable.
    @return The pointer to the data item object. */
   const DataItem * Package::getPayloadObject() const {
      auto item = std::get_if<std::unique_ptr<DataItem>>(&payload);
      if (item) {
         return item->get();
      }
      return nullptr;
   }
   
   /** Use for getting the modifiable pointer to the parsed,
    structured DataItem of the Package. May be null if there
    is no data or it has not been parsed from the data member variable.
    @return The pointer to the data item object. */
   DataItem * Package::getPayloadObject() {
      auto item = std::get_if<std::unique_ptr<DataItem>>(&payload);
      if (item) {
         return item->get();
      }
      return nullptr;
   }
   
   /** Sets the new data item for this package, deleting the old one (if any).
    @param item The new dataitem for this package. DataItem is copied, so caller must handle
    the parameter object lifetime. */
   void Package::setPayload(std::unique_ptr<DataItem> item) {
      payload = std::move(item);
   }
   
   /** Use to query if package is empty. Package is empty if it has no type and dataItem is nullptr.
    @return Returns true if package is empty. */
   bool Package::isEmpty() const {
      return (type == NoType);
   }
   
   /** Sets the address where the package came from. Empty, if package did not come from outside the node.
    @param o Address of the package origin. */
   void Package::setOrigin(const std::string & o) {
      originAddress = o;
   }

   /** Gets the address where the package came from. Empty, if package did not come from outside the node.
    @return Address of the package origin. */
   const std::string & Package::origin() const {
      return originAddress;
   }

   /** Use to query if the package has an origin address or not.
    @return Returns true if the package has an origin address. */
   bool Package::hasOrigin() const {
      return originAddress.length() > 0;
   }

   /** Sets the package's destination address.
    @param d The destination of the package. */
   void Package::setDestination(const std::string & d) {
      destinationAddress = d;
   }

   /** Gets the package's destination address.
    @return The destination of the package. */
   const std::string & Package::destination() const {
      return destinationAddress;
   }

   /** Use to query if the package has a package specific destination address or not.
    @return Returns true if the package has a destination address. */
   bool Package::hasDestination() const {
      return destinationAddress.length() > 0;
   }

   const Package & Package::operator = (const Package & p) {
      if (this != &p) {
         uid = p.uid;
         type = p.type;
         setPayloadVariant(p.payload);
         originAddress = p.originAddress;
         destinationAddress = p.destinationAddress;
      }
      return *this;
   }
   
   void Package::setPayloadVariant(const std::variant<std::string,std::unique_ptr<DataItem>> & x) {
      auto item = std::get_if<std::unique_ptr<DataItem>>(&x);
      if (item) {
         setPayload(item->get()->clone());
      } else {
         setPayload(std::get<std::string>(x));
      }

   }
   
   const Package & Package::operator = (Package && p) {
      if (this != &p) {
         uid = std::move(p.uid);
         type = std::move(p.type);
         payload = std::move(p.payload);
         originAddress = std::move(p.originAddress);
         destinationAddress = std::move(p.destinationAddress);
      }
      return *this;
   }
   
   
   bool Package::operator == (const Package & pkg) const {
      return (uid == pkg.uid);
   }
      
   /**
    Externalizes the Package to a JSON object. Note that (at least currently) the originAddress and
    destinationAddress are not externalized to JSON. Addresses are used only by package handlers,
    NetworkReader and NetworkWriter to route packages.
    @param j The JSON object fill with package contents.
    @param package The package to exernalize.
    */
   void to_json(nlohmann::json & j, const Package & package) {
      j = nlohmann::json{{"package", to_string(package.getUuid())}};
      j["type"] = package.getTypeAsString();
      j["payload"] = package.getPayloadString();
   }
   
   /**
    Internalizes the package contents from a JSON structure. Note that (at least currently) the originAddress and
    destinationAddress are not internalized from JSON. Addresses are used only by package handlers,
    NetworkReader and NetworkWriter to route packages.
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
         package.setPayload(j["payload"].get<std::string>());
      }
   }
   
   
   
} //namespace
