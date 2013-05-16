* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-16 Markus Litz <Markus.Litz@dlr.de>
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
/**
   @file Stubs implement simple stubs to support unitTests.
*/


char* curlGetURLInMemory_stub(const char *url)
{
    if (strcmp(url, "http://www.w3schools.com/XML/note.xml") == 0) {
                return "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><a></a>"
    }
}

