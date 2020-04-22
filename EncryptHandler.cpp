//
//  EncryptHandler.cpp
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 21.04.2020.
//  Copyright (c) 2013-2020 Antti Juustila. All rights reserved.
//

#include <g3log/g3log.hpp>

#include <ProcessorNode/Package.h>

#include <ProcessorNode/EncryptHandler.h>

namespace OHARBase {

const std::string EncryptHandler::TAG{"EncryptHandler "};

/**
 Constructs a EncryptHandler object.
 @param myNode The processor node to use to forward the ping message.
 */
EncryptHandler::EncryptHandler(EncryptHandler::Mode mode)
: encryptionMode(mode)
{
}

EncryptHandler::~EncryptHandler() {
   
}

/**
 Implementation of encrpytion and decryption of the message using rot13.
 @param data Handler checks if the data contains a data message and if yes, handles it.
 @return Returns false to let other handlers consume the package.
 */
bool EncryptHandler::consume(OHARBase::Package & package) {
   if (package.getType() == OHARBase::Package::Data) {
      const std::string jsonString = package.getPayloadString();
      if (jsonString.length() > 0) {
         std::string result;
         // With rot13, both encrypt and decrypt do the same thing, just want to show here that there are two ops to do,
         // with other methods this would be two different things done.
         if (encryptionMode == Mode::Encrypt) {
            rot13(jsonString, result);
            package.setPayload(result);
         } else if (encryptionMode == Mode::Decrypt) {
            rot13(jsonString, result);
            package.setPayload(result);
         }
         LOG(INFO) << TAG << "Changed " << jsonString << " to " << result;
      } else {
         LOG(INFO) << TAG << "No data to encrypt or  decrypt";
      }
   }
   return false;
}

// Adapted from OhRa exercise work Created by Matias Kinnunen on 2.5.2018, LGPL licensed.
void EncryptHandler::rot13(const std::string & source, std::string & destination) {
   for (const char& c : source) {
      int charCode = (int) c;
      if ((c >= 65 && c <= 77) || (c >= 97 && c <= 109)) {
         destination += (char) (charCode + 13);
         continue;
      }
      
      // N-Z or n-z
      if ((c >= 78 && c <= 90) || (c >= 110 && c <= 122)) {
         destination += (char) (charCode - 13);
         continue;
      }
      destination += c;
   }
}

} //namespace


