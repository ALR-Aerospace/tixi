/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: add_attribute_check.cpp 176 2012-10-08 21:20:23Z markus.litz $
*
* Version: $Revision: 176 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
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
    @test Tests for vector routines.
*/

static TixiDocumentHandle documentHandleGet = -1;
static TixiDocumentHandle documentHandleAdd = -1;
static TixiDocumentHandle tmpHandle = -1;

class VectorTests : public ::testing::Test {
 protected:
  virtual void SetUp() {
     const char* xmlFilenameGet = "TestData/vectorcount.xml";
     const char* xmlFilenameAdd = "TestData/vectoradd.xml";
     ASSERT_TRUE( tixiOpenDocument( xmlFilenameGet, &documentHandleGet ) == SUCCESS);
     ASSERT_TRUE( tixiOpenDocument( xmlFilenameAdd, &documentHandleAdd  ) == SUCCESS);
  }

  virtual void TearDown() {
      ASSERT_TRUE ( tixiCloseDocument( documentHandleGet ) == SUCCESS );
      ASSERT_TRUE ( tixiCloseDocument( documentHandleAdd ) == SUCCESS );
  }
};

TEST_F(VectorTests, tixiVectorGetTests)
{
    int count = 0;
    double *allPoints = NULL;

    // check sizes
    ASSERT_TRUE ( tixiGetVectorSize(documentHandleGet, "/a/aeroPerformanceMap/cfx", &count) == SUCCESS );
    ASSERT_TRUE ( count == 32 );
    ASSERT_TRUE ( tixiGetVectorSize(documentHandleGet, "/a/aeroPerformanceMap/cfy", &count) == ELEMENT_NOT_FOUND );
    ASSERT_TRUE ( tixiGetVectorSize(documentHandleGet, "/a/aeroPerformanceMap/cfz", &count) == SUCCESS );
    ASSERT_TRUE ( count == 2 );

    // check values returned
    count = 10;
    ASSERT_TRUE ( tixiGetFloatVector(documentHandleGet, "/a/aeroPerformanceMap/cfx", &allPoints, count) == SUCCESS );
    ASSERT_TRUE ( allPoints[0] == 1.0 ); printf("\n\n==>%f\n\n", allPoints[0]);
    ASSERT_TRUE ( allPoints[7] == 8.0 );
}

TEST_F(VectorTests, tixiVectorAddTests)
{
    int count = 0;
    double points[10] = {1, 4, 5.8, 77.0, 5, 6, 7, 8, 9, 10};
    double *newPoints = NULL;
    const char* xmlOutName = "x.xml";
    count = 3;

    // write parts of the array to an intermediate file
    ASSERT_TRUE ( tixiAddFloatVector(documentHandleAdd, "/a", "test", points, count) == SUCCESS );
    ASSERT_TRUE ( tixiSaveDocument(documentHandleAdd, xmlOutName) == SUCCESS );

    // read in again and check values
    ASSERT_TRUE ( tixiOpenDocument( xmlOutName, &tmpHandle ) == SUCCESS );
    ASSERT_TRUE ( tixiGetFloatVector(tmpHandle, "/a/test", &newPoints, count) == SUCCESS );
    ASSERT_TRUE ( (newPoints[0] == 1) && (newPoints[1] == 4) && (newPoints[2] == 5.8) );
    ASSERT_TRUE ( tixiCloseDocument( tmpHandle ) == SUCCESS );
}
