//
//  Package.h
//
//  Created by Antti Juustila on 28.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#ifndef __PipesAndFiltersNode__Package__
#define __PipesAndFiltersNode__Package__

#include <string>
#include <variant>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <nlohmann/json.hpp>


namespace OHARBase {
   
   // Forward declaration.
   class DataItem;
   
   /**
    Package encapsulates the data send between the ProcessorNode objects as well
    as passing the data around within a ProcessorNode and the Handlers and readers/writers
    in the node.<p>
    Each package has a unique identifier which can be used to identify data packages.
    Package also has a type, indicating is the package containing actual data to be
    handled, or is the package containing a control message. Control messages influence
    on the way the Node behaves. These include e.g. shutting down the node.
    @todo Add a "corrupt" flag for packages that have bad content. These packages
    are just passed through the Filters, logging their presence, and they are not
    handled by any Handler. Would aid in debugging/testing since corrupt packages would
    not just disappear like a fart in the Sahara.
    @author Antti Juustila
    */
   class Package final {
   public:
      /*! The type of the package. */
      enum Type {
         NoType,   /*!< The package has no type (yet); it is uninitialized. */
         Control,  /*!< The package contains a control message. */
         Data,      /*!< The package contains actual application specific data. */
         Configuration /*!< The package contains node configuration data or commands. */
      };
      
      Package();
      Package(const Package & p);
      Package(Package && p);
      Package(const boost::uuids::uuid & id);
      Package(Type ptype, const std::string & data);
      Package(const boost::uuids::uuid & id, Type ptype, const std::string & data);
      ~Package();
      
      const boost::uuids::uuid & getUuid() const;
      void setUuid(const boost::uuids::uuid & id);
      Type getType() const;
      void setType(Type ptype);
      const std::string & getPayloadString() const;
      void setPayload(const std::string & d);
      const DataItem * getPayloadObject() const;
      DataItem * getPayloadObject();
      void setPayload(std::unique_ptr<DataItem> item);

      void setOrigin(const std::string & o);
      const std::string & origin() const;
      bool hasOrigin() const;

      void setDestination(const std::string & d);
      const std::string & destination() const;
      bool hasDestination() const;
      
      bool isEmpty() const;
      const Package & operator = (const Package & p);
      const Package & operator = (Package && p);
      bool operator == (const Package & pkg) const;
            
      const std::string & getTypeAsString() const;
      void setTypeFromString(const std::string & typeStr);
      
   private:
      void setPayloadVariant(const std::variant<std::string,std::unique_ptr<DataItem>> & x);

   private:
      /** The unique identifier for a package. Generated using the boost library support. */
      boost::uuids::uuid uid;
      /** Type of the data package. It is either Control or Data. Control packages are
       commands to both ProcessorNode and DataHandler objects on how they are controlled.
       Commands are:<br />
       <ul><li>ping, which causes the Node to print out a ping command,</li>
       <li>readfile, which causes the Node to read it's data file if it has been configured to do so,</li>
       <li>shutdown, which causes the node to shut down itself in a controlled way.</li></ul>
       All the commands are also forwarded to the next Node in the Pipes & Filters architecture.
       Thus all Nodes can be shut down by writing the command "shutdown" in the console of the first Node.<p>
       Data packages are application specific data items. */
      Type type;
      
      /** Data as received from the network/sent to the network, either as JSON string or
       parsed DataItem object pointer in managed pointer. Default value is an emptry string.
       A c++17 std::variant holds either the JSON string or the parsed DataItem object.
       Contents of DataItem is application specific. Application developers
       subclass their data objects from DataItem and implement application specific data
       structures in their subclasses. Parsing of data from string to DataItem
       happens in other application specific classes. */
      std::variant<std::string, std::unique_ptr<DataItem>> payload;

      /** Origin address of the package. */
      std::string originAddress;
      
      /** Destination address of the packet. Empty if the Node's default destination address is to be used. */
      std::string destinationAddress;
      
      /** Textual representation of the package type Package::Control. */
      static const std::string controlStr;
      /** Textual representation of the package type Package::Data. */
      static const std::string dataStr;
      /** Textual representation of the package type Package::Configuration. */
      static const std::string configurationStr;
      /** Textual representation of the package type Package:NoType. */
      static const std::string emptyString;
   };
   
   void to_json(nlohmann::json & j, const Package & package);
   void from_json(const nlohmann::json & j, Package & package);
   
   
} //namespace
#endif /* defined(__PipesAndFiltersNode__Package__) */
