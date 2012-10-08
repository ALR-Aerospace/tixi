/* 
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: xpaths.c 126 2012-08-12 14:55:28Z martinsiggel@gmail.com $ 
*
* Version: $Revision: 126 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "test.h" // Brings in the GTest framework

#include "tixi.h"


/** 
    @test Tests for adding composite elements.
*/   

static TixiDocumentHandle documentHandle = -1;

class CompositeTests : public ::testing::Test {
 protected:
  virtual void SetUp() {
        char* rootElementName = "root";
        ASSERT_TRUE( tixiCreateDocument( rootElementName, &documentHandle ) == SUCCESS);
  }
  
  // virtual void TearDown() {}
};


TEST_F(CompositeTests, add_composits) 
{
#define N_VALUES 5

    double values[N_VALUES] = {0., 1., 2., 3., 4.};
    char* attributes[N_VALUES] = { "a", "b", "c", "d", "e"};
    char* parentPath = "/root";
    char* listName = "numbers";
    char* childName = "number";
    char* format = "%08.2f";
    char* childAttributeName = "letter";
    int nValues = N_VALUES;

    ASSERT_TRUE( tixiAddDoubleListWithAttributes( documentHandle, parentPath,  listName, childName, childAttributeName, 
                                           values,  format,  attributes,  nValues ) == SUCCESS );

    ASSERT_TRUE( tixiSaveDocument( documentHandle, "list.xml" )== SUCCESS);
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS);
}
