/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
    UID Checks
*/


const char* filename_uid_correct = "TestData/uid_correct.xml";
const char* filename_uid_duplicated = "TestData/uid_duplicated.xml";
const char* filename_without_uids = "TestData/uid_no_uids.xml";
const char* filename_links_broken = "TestData/uid_broken_link.xml";
const char* filename_links = "TestData/uid_link.xml";


TEST(uid_checks, tixiUIDCheck_ok)
{
    TixiDocumentHandle documentHandle = -1;
    ASSERT_TRUE( tixiOpenDocument( filename_uid_correct, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDCheckDuplicates( documentHandle ) == SUCCESS);
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );
}

TEST(uid_checks, tixiUIDCheck_duplicates)
{
    TixiDocumentHandle documentHandle = -1;
    ASSERT_TRUE( tixiOpenDocument( filename_uid_duplicated, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDCheckDuplicates( documentHandle ) == UID_NOT_UNIQUE);
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );

    /* check a document without uids */
    ASSERT_TRUE( tixiOpenDocument( filename_without_uids, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDCheckDuplicates( documentHandle ) == SUCCESS);
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );
}

TEST(uid_checks, tixiUIDCheck_getXPath)
{
    TixiDocumentHandle documentHandle = -1;
    char *xpath = NULL;

    ASSERT_TRUE( tixiOpenDocument( filename_uid_correct, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDGetXPath( documentHandle, "b", &xpath ) == SUCCESS );
    ASSERT_TRUE( !strcmp(xpath, "/root/a/b") == SUCCESS );
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );
}

TEST(uid_checks, tixiUIDCheck_getXPathFailed)
{
    TixiDocumentHandle documentHandle = -1;
    char *xpath = NULL;

    ASSERT_TRUE( tixiOpenDocument( filename_uid_correct, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDGetXPath( documentHandle, "bc", &xpath ) == SUCCESS );
    ASSERT_TRUE( strlen(xpath) == 0 );
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );
}

TEST(uid_checks, tixiUIDCheck_checkExists)
{
    TixiDocumentHandle documentHandle = -1;

    ASSERT_TRUE( tixiOpenDocument( filename_uid_correct, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDCheckExists( documentHandle, "schlumpf" ) == SUCCESS );
    ASSERT_TRUE( tixiUIDCheckExists( documentHandle, "i_do_not_exist" ) == UID_DONT_EXISTS );
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );
}

TEST(uid_checks, tixiUIDCheck_setToXpath)
{
    TixiDocumentHandle documentHandle = -1;
    char *text = NULL;
    const char *new_uid_text = "new_uid_text";

    ASSERT_TRUE( tixiOpenDocument( filename_uid_correct, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDSetToXPath(documentHandle, "/root/a[2]/b[1]", new_uid_text) == SUCCESS );
    ASSERT_TRUE( tixiGetTextAttribute(documentHandle, "/root/a[2]/b[1]", "uID", &text) == SUCCESS );
    ASSERT_TRUE( !strcmp(text, new_uid_text) );
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );
}

TEST(uid_checks, tixiUIDCheck_checkLink)
{
    TixiDocumentHandle documentHandle = -1;
    char *text = NULL;

    ASSERT_TRUE( tixiOpenDocument( filename_links_broken, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDCheckLinks(documentHandle) == UID_LINK_BROKEN );
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );

    ASSERT_TRUE( tixiOpenDocument( filename_links, &documentHandle ) == SUCCESS );
    ASSERT_TRUE( tixiUIDCheckLinks(documentHandle) == SUCCESS );
    ASSERT_TRUE( tixiCloseDocument( documentHandle ) == SUCCESS );
}

