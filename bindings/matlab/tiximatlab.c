/* 
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2012-08-13 Martin Siggel <martin.siggel@dlr.de>
* Changed: $Id: tiximatlab.c 122 2012-08-10 07:35:30Z martinsiggel@gmail.com $ 
*
* Version: $Revision: 122 $
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

#include <math.h>
#include <matrix.h>
#include <mex.h>
#include <tixi.h>

#include <string.h>
#include <stdio.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32)
#define MEX_EXPORT __declspec(dllexport)
#endif

/* Visual Studio C macros */
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/*
 * helping function to convert matlab array into c-string 
 */
int mxToString(const mxArray * mxStr, char ** cstr){
  size_t strLen = 0;

       /* Input must be a string. */
  if (mxIsChar(mxStr) != 1){
    mexErrMsgTxt("Input must be a string.");
    return 1;
  }

  /* Input must be a row vector. */
  if (mxGetM(mxStr) != 1){
    mexErrMsgTxt("Input must be a row vector.");
    return 1;
  }
    
  /* Get the length of the input string. */
  strLen = (mxGetM(mxStr) * mxGetN(mxStr)) + 1;

  /* Allocate memory for input and output strings. */
  *cstr = (char*) mxCalloc(strLen, sizeof(char));

  /* Copy the string data from prhs[0] into a C string 
   * input_buf. */
  return mxGetString(mxStr, *cstr, strLen);
}


void handleTixiError(ReturnCode code){
    switch(code){
case SUCCESS:
        break;
case FAILED:
        mexErrMsgTxt("Unspecified error\n");
        break;
case INVALID_XML_NAME:
        mexErrMsgTxt("Invalid xml file name\n");
        break;
case NOT_WELL_FORMED:
        mexErrMsgTxt("Document is not wellformed\n");
        break;
case NOT_SCHEMA_COMPLIANT:
        mexErrMsgTxt("Document is not schema compliant\n");
        break;
case NOT_DTD_COMPLIANT:
        mexErrMsgTxt("Document is not DTD compliant\n");
        break;
case INVALID_HANDLE:
        mexErrMsgTxt("Document handle is not valid\n");
        break;
case INVALID_XPATH:
        mexErrMsgTxt("XPath expression is not valid");
        break;
case ELEMENT_NOT_FOUND:
        mexErrMsgTxt("Element does not exist in document");
        break;
case INDEX_OUT_OF_RANGE:
        mexErrMsgTxt("Index supplied as argument is not inside the admissible range\n");
        break;
case NO_POINT_FOUND:
        mexErrMsgTxt("No point element found a given XPath\n");
        break;
case NOT_AN_ELEMENT:
        mexErrMsgTxt("XPath expression does not point to an XML-element node");
        break;
case ATTRIBUTE_NOT_FOUND:
        mexErrMsgTxt("Element does not have the attribute\n");
        break;
case OPEN_FAILED:
        mexErrMsgTxt("Error on opening the file\n");
        break;
case OPEN_SCHEMA_FAILED:
        mexErrMsgTxt(" Error on opening the schema file\n");
        break;
case OPEN_DTD_FAILED:
        mexErrMsgTxt("Error on opening the DTD file\n");
        break;
case CLOSE_FAILED:
        mexErrMsgTxt("Error on closing the file\n");
        break;
case ALREADY_SAVED:
        mexErrMsgTxt("Trying to modify already saved document\n");
        break;
case ELEMENT_PATH_NOT_UNIQUE:
        mexErrMsgTxt("Path expression can not be resolved unambiguously\n");
        break;
case NO_ELEMENT_NAME:
        mexErrMsgTxt("Element name argument is NULL\n");
        break;
case NO_CHILDREN:
        mexErrMsgTxt("Node has no children\n");
        break;
case CHILD_NOT_FOUND:
        mexErrMsgTxt("Named child is not child of element specified\n");
        break;
case EROROR_CREATE_ROOT_NODE:
        mexErrMsgTxt("Error when adding root node to new document\n");
        break;
case DEALLOCATION_FAILED:
        mexErrMsgTxt("On closing a document the deallocation of allocated memory fails\n");
        break;
case NO_NUMBER:
        mexErrMsgTxt("No number specified\n");
        break;
case NO_ATTRIBUTE_NAME:
        mexErrMsgTxt("No attribute name specified\n");
        break;
case STRING_TRUNCATED:
        mexErrMsgTxt("String variable supplied is to small to hold the result\n");
        break;
case NON_MATCHING_NAME:
        mexErrMsgTxt("Row or column name specified do not match the names used in the document\n");
        break;
case NON_MATCHING_SIZE:
        mexErrMsgTxt("Number of rows or columns specified do not match the sizes of the matrix in the document\n");
        break;
case MATRIX_DIMENSION_ERROR:
        mexErrMsgTxt("Supplied matrix dimensions are smaller than 1\n");
        break;
case COORDINATE_NOT_FOUND:
        mexErrMsgTxt("Missing coordinate inside a point element\n");
        break;
case UNKNOWN_STORAGE_MODE:
        mexErrMsgTxt("Storage mode specified is neither ROW_WISE nor COLUMN_WISE\n");
        break;
case UID_NOT_UNIQUE:
        mexErrMsgTxt("One or more uID's are not unique\n");
        break;
case UID_DONT_EXISTS:
        mexErrMsgTxt("A given uID does not exist\n");
        break;
case UID_LINK_BROKEN:
        mexErrMsgTxt("A specified Link has no correspoding uid in that data set\n");
        break;
default:
        mexErrMsgTxt("An unknown error occured\n");
    }
}

int mxToInt(const mxArray * mxArr){
    double dv = *mxGetPr(mxArr);
    if(!isinteger(dv))
        mexWarnMsgTxt("Argument is not an integer\n");

    return (int)dv;
}

void mex_tixiAddHeader(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * toolName = NULL;
    char * version = NULL;
    char * authorName = NULL;
    TixiDocumentHandle handle = 0;

    if(nrhs != 5){
        mexErrMsgTxt("tixiAddHeader(handle, toolName, version, authorName): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid toolName argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid version argument\n");

    if(!mxIsChar(prhs[4]))
        mexErrMsgTxt("Invalid authorName argument\n");

    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&toolName);
    mxToString(prhs[3],&version);
    mxToString(prhs[4],&authorName);

    handleTixiError(tixiAddHeader(handle, toolName, version, authorName));
}

void mex_tixiAddCpacsHeader(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * name = NULL;
    char * version = NULL;
    char * creator = NULL;
    char * description = NULL;
    TixiDocumentHandle handle = 0;

    if(nrhs != 6)
        mexErrMsgTxt("tixiAddCpacsHeader(handle, name, creator, version, description): Wrong number of arguments\n");

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid name argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid creator argument\n");

    if(!mxIsChar(prhs[4]))
        mexErrMsgTxt("Invalid version argument\n");

    if(!mxIsChar(prhs[5]))
        mexErrMsgTxt("Invalid description argument\n");



    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&name);
    mxToString(prhs[3],&creator);
    mxToString(prhs[4],&version);
    mxToString(prhs[5],&description);


    handleTixiError(tixiAddCpacsHeader(handle, name, creator, version, description));
}

void mex_tixiOpenDocument(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * filename = NULL;
    TixiDocumentHandle handle;

    if(nrhs != 2){
        mexErrMsgTxt("tixiOpenDocument(filename): Wrong number of arguments\n");
    }

    // get the filename
    mxToString(prhs[1],&filename);
    handleTixiError(tixiOpenDocument(filename, &handle));

    // return the handle
    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = handle;
}

void mex_tixiOpenDocumentRecursive(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * filename = NULL;
    OpenMode mode = OPENMODE_PLAIN;
    TixiDocumentHandle handle;

    if(nrhs != 3){
        mexErrMsgTxt("tixiOpenDocument(filename, openMode): Wrong number of arguments\n");
    }

    // get the filename
    mxToString(prhs[1],&filename);

    // get openmode
    mode = (OpenMode) mxToInt(prhs[2]);
    if(mode != OPENMODE_PLAIN && mode != OPENMODE_RECURSIVE){
        mexErrMsgTxt("Invalid open mode\n");
    }

    handleTixiError(tixiOpenDocumentRecursive(filename, &handle, mode));

    // return the handle
    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = handle;
}

void mex_tixiOpenDocumentFromHTTP(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * url = NULL;
    TixiDocumentHandle handle;

    if(nrhs != 2){
        mexErrMsgTxt("tixiOpenDocumentFromHTTP(filename): Wrong number of arguments\n");
    }

    // get the url
    mxToString(prhs[1],&url);
    handleTixiError(tixiOpenDocumentFromHTTP(url, &handle));

    // return the handle
    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = handle;
}

void mex_tixiCreateDocument(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * rootName = NULL;
    TixiDocumentHandle handle;

    if(nrhs != 2){
        mexErrMsgTxt("tixiCreateDocument(rootElementName): Wrong number of arguments\n");
    }

    // get the root name
    mxToString(prhs[1],&rootName);
    handleTixiError(tixiCreateDocument(rootName, &handle));

    // return the handle
    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = handle;
}


void mex_tixiCloseDocument(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    const mxArray * mxHandle = prhs[1];
    int handle = -1;
    if(nrhs != 2){
        mexErrMsgTxt("tixiCloseDocument(handle): Wrong number of arguments\n");
    }

    if(mxGetM(mxHandle)!= 1 || mxGetN(mxHandle) != 1){
        mexErrMsgTxt("Wrong handle format\n");
    }

    // get handle
    handle = (int) *mxGetPr(mxHandle);
    handleTixiError(tixiCloseDocument(handle));
}

void mex_tixiSchemaValidateFromFile(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xsdFilename = NULL;
    int handle = -1;
    if(nrhs != 3){
        mexErrMsgTxt("tixiSchemaValidateFromFile(handle, xsdFilename): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xdsFilename argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xsdFilename);
    handleTixiError(tixiSchemaValidateFromFile(handle, xsdFilename));
}

void mex_tixiSchemaValidateFromString(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * string = NULL;
    int handle = -1;
    if(nrhs != 3){
        mexErrMsgTxt("tixiSchemaValidateFromString(handle, string): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid string argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&string);
    handleTixiError(tixiSchemaValidateFromString(handle, string));
}

void mex_tixiDTDValidate(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * DTDFilename = NULL;
    int handle = -1;
    if(nrhs != 3){
        mexErrMsgTxt("tixiDTDValidate(handle, DTDFilename): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid DTDFilename argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&DTDFilename);
    handleTixiError(tixiDTDValidate(handle, DTDFilename));
}


void mex_tixiGetTextElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    int handle = -1;
    char * text;
    if(nrhs != 3){
        mexErrMsgTxt("tixiGetTextElement(handle, xpath): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    handleTixiError(tixiGetTextElement(handle, xpath, &text));

    plhs[0] = mxCreateString(text);
}

void mex_tixiGetDoubleElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    int handle = -1;
    double d;
    if(nrhs != 3){
        mexErrMsgTxt("tixiGetDoubleElement(handle, xpath): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    handleTixiError(tixiGetDoubleElement(handle, xpath, &d));

    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = d;
}

void mex_tixiGetIntegerElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    int handle = -1;
    int i;
    if(nrhs != 3){
        mexErrMsgTxt("tixiGetIntegerElement(handle, xpath): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    handleTixiError(tixiGetIntegerElement(handle, xpath, &i));

    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = (double)i;
}

void mex_tixiGetBooleanElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    int handle = -1;
    int b;
    if(nrhs != 3){
        mexErrMsgTxt("tixiGetBooleanElement(handle, xpath): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    handleTixiError(tixiGetBooleanElement(handle, xpath, &b));

    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = b;
}

void mex_tixiUpdateTextElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * text = NULL;
    int handle = -1;
    if(nrhs != 4){
        mexErrMsgTxt("tixiUpdateTextElement(handle, xpath, text): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid text argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    mxToString(prhs[3],&text);
    handleTixiError(tixiUpdateTextElement(handle, xpath, text));
}

void mex_tixiUpdateDoubleElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * format = NULL;
    double d = 0;
    int handle = -1;
    if(nrhs != 5){
        mexErrMsgTxt("tixiUpdateDoubleElement(handle, xpath, value, format): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");

    if(!isscalar(prhs[3]))
        mexErrMsgTxt("Invalid value argument\n");

    if(!mxIsChar(prhs[4]))
        mexErrMsgTxt("Invalid format argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    
    d = *mxGetPr(prhs[3]);

    mxToString(prhs[4],&format);
    handleTixiError(tixiUpdateDoubleElement(handle, xpath, d, format));
}

void mex_tixiAddTextElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * name = NULL;
    char * text = NULL;
    int handle = -1;
    if(nrhs != 5){
        mexErrMsgTxt("tixiAddTextElement(handle, parentPath, elementName, text): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid name argument\n");

    if(!mxIsChar(prhs[4]))
        mexErrMsgTxt("Invalid text argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    mxToString(prhs[3],&name);
    mxToString(prhs[4],&text);
    handleTixiError(tixiAddTextElement(handle, xpath, name, text));
}

void mex_tixiAddTextElementAtIndex(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * name = NULL;
    char * text = NULL;
    int index = 0;
    int handle = -1;
    if(nrhs != 6){
        mexErrMsgTxt("tixiAddTextElementAtIndex(handle, parentPath, elementName, text, index): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid name argument\n");

    if(!mxIsChar(prhs[4]))
        mexErrMsgTxt("Invalid text argument\n");

    if(!isscalar(prhs[5]))
        mexErrMsgTxt("Invalid index argument\n");

    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    mxToString(prhs[3],&name);
    mxToString(prhs[4],&text);
    index = mxToInt(prhs[5]);
    handleTixiError(tixiAddTextElementAtIndex(handle, xpath, name, text,index));
}


void mex_tixiSaveDocument(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * filename = NULL;
    int handle = -1;
    if(nrhs != 3){
        mexErrMsgTxt("tixiSaveDocument(handle, filename): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid filename argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&filename);
    handleTixiError(tixiSaveDocument(handle, filename));
}

void mex_tixiSaveCompleteDocument(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * filename = NULL;
    int handle = -1;
    if(nrhs != 3){
        mexErrMsgTxt("tixiSaveCompleteDocument(handle, filename): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid filename argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&filename);
    handleTixiError(tixiSaveCompleteDocument(handle, filename));
}

void mex_tixiSaveAndRemoveDocument(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * filename = NULL;
    int handle = -1;
    if(nrhs != 3){
        mexErrMsgTxt("tixiSaveAndRemoveDocument(handle, filename): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid filename argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&filename);
    handleTixiError(tixiSaveAndRemoveDocument(handle, filename));
}

void mex_tixiExportDocumentAsString(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * str = NULL;
    int handle = -1;
    if(nrhs != 2)
         mexErrMsgTxt("tixiExportDocumentAsString(handle): Wrong number of arguments\n");

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    handle = mxToInt(prhs[1]);
    handleTixiError(tixiExportDocumentAsString(handle, &str));

    plhs[0] = mxCreateString(str);
}


void mex_tixiImportFromString(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * string = NULL;
    TixiDocumentHandle handle;

    if(nrhs != 2){
        mexErrMsgTxt("tixiImportFromString(string): Wrong number of arguments\n");
    }

    // get the string
    mxToString(prhs[1],&string);
    handleTixiError(tixiImportFromString(string, &handle));

    // return the handle
    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = handle;
}

void mex_tixiGetVectorSize(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * path = NULL;
    int handle = 0;
    int size = 0;
    if(nrhs != 3){
        mexErrMsgTxt("tixiGetVectorSize(handle, vectorPath): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid vectorPath argument\n");

    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&path);
    handleTixiError(tixiGetVectorSize(handle, path, &size));

    // return the size
    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = (double) size;
}

void mex_tixiCreateElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * name = NULL;
    int handle = -1;
    if(nrhs != 4){
        mexErrMsgTxt("tixiCreateElement(handle, parentPath, elementName): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid element name argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    mxToString(prhs[3],&name);
    handleTixiError(tixiCreateElement(handle, xpath, name));
}


void mex_tixiCreateElementAtIndex(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * name = NULL;
    char * text = NULL;
    int index = 0;
    int handle = -1;
    if(nrhs != 5){
        mexErrMsgTxt("tixiCreateElementAtIndex(handle, parentPath, elementName, index): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid name argument\n");

    if(!isscalar(prhs[4]))
        mexErrMsgTxt("Invalid index argument\n");

    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    mxToString(prhs[3],&name);
    index = mxToInt(prhs[4]);
    handleTixiError(tixiCreateElementAtIndex(handle, xpath, name,index));
}

void mex_tixiRemoveElement(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    int handle = -1;
    if(nrhs != 3){
        mexErrMsgTxt("tixiRemoveElement(handle, xpath): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    handleTixiError(tixiRemoveElement(handle, xpath));
}

void mex_tixiGetFloatVector(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * path = NULL;
    int handle = 0;
    int size = 0;
    int i = 0;
    double * array = NULL;
    double * outArray = NULL;
    if(nrhs != 4){
        mexErrMsgTxt("tixiGetFloatVector(handle, vectorPath, numValues): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid vectorPath argument\n");

    if(!isscalar(prhs[3]))
        mexErrMsgTxt("Invalid vector size argument!\n");

    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&path);
    size = mxToInt(prhs[3]);

    if(size < 1)
        mexErrMsgTxt("Invalid number of elements\n");

    handleTixiError(tixiGetFloatVector(handle, path, &array, size));

    // return the vector
    plhs[0] = mxCreateDoubleMatrix(size,1, mxREAL);
    outArray = mxGetPr(plhs[0]);
    for(i = 0; i < size; ++i)
        outArray[i] = array[i];
}

void mex_tixiAddFloatVector(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * path = NULL;
    char * name = NULL;
    int handle = 0;
    size_t size = 0;
    const mxArray * mVec = NULL;
    if(nrhs != 5){
        mexErrMsgTxt("tixiAddFloatVector(handle, parentPath, vectorName, vector): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1]))
        mexErrMsgTxt("Invalid Handle!\n");

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid parentPath argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid vectorName argument\n");

    if(mxIsChar(prhs[4]))
        mexErrMsgTxt("Invalid float vector\n");

    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&path);
    mxToString(prhs[3],&name);

    mVec = prhs[4];
    if(mxGetN(mVec) != 1 && mxGetM(mVec)!=1)
        mexErrMsgTxt("Dimension mismatch of input vector");

    size = mxGetN(mVec)*mxGetM(mVec);

    if(size < 1)
        mexErrMsgTxt("Invalid number of elements\n");

    handleTixiError(tixiAddFloatVector(handle, path, name, mxGetPr(mVec), (int) size));
}


void mex_tixiGetNamedChildrenCount(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * childName = NULL;
    int handle = -1;
    int count = 0;
    if(nrhs != 4){
        mexErrMsgTxt("tixiGetNamedChildrenCount(handle, xpath, childName): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid xpath argument\n");

    if(!mxIsChar(prhs[3]))
        mexErrMsgTxt("Invalid child name\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);
    mxToString(prhs[3],&childName);
    handleTixiError(tixiGetNamedChildrenCount(handle, xpath, childName, &count));

    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    *mxGetPr(plhs[0]) = (double) count;
}

void mex_tixiAddPoint(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    char * xpath = NULL;
    char * format = NULL;
    double x = 0., y = 0., z = 0.;
    int handle = -1;
    const mxArray * mpoint = NULL;
    if(nrhs != 5){
        mexErrMsgTxt("tixiAddPoint(handle, parentPath, point, format): Wrong number of arguments\n");
    }

    if(!isscalar(prhs[1])){
        mexErrMsgTxt("Invalid Handle!\n");
    }

    if(!mxIsChar(prhs[2]))
        mexErrMsgTxt("Invalid parentPath argument\n");

    //check dimension of point (has to be 3)
    mpoint = prhs[3];
    if(mxGetM(mpoint)*mxGetN(mpoint) !=3)
        mexErrMsgTxt("Number of dimensions of point argument has to be 3.");

    if(!mxIsChar(prhs[4]))
        mexErrMsgTxt("Invalid format argument\n");


    handle = mxToInt(prhs[1]);
    mxToString(prhs[2],&xpath);

    x = mxGetPr(mpoint)[0];
    y = mxGetPr(mpoint)[1];
    z = mxGetPr(mpoint)[2];

    mxToString(prhs[4],&format);
    handleTixiError(tixiAddPoint(handle, xpath, x, y, z, format));
}

/*
 * main entry point for MATLAB
 * deals as a dispatcher here
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int status = 0;
    char * functionName = NULL;
    if(nrhs < 1){
        mexErrMsgTxt("No function argument given!");
        return;
    }
    
  mxToString(prhs[0],&functionName);

  if(strcmp(functionName,"tixiOpenDocument")==0){
      mex_tixiOpenDocument(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiOpenDocumentRecursive")==0){
      mex_tixiOpenDocumentRecursive(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiOpenDocumentFromHTTP")==0){
      mex_tixiOpenDocumentFromHTTP(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiCreateDocument")==0){
      mex_tixiCreateDocument(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiCloseDocument")==0){
      mex_tixiCloseDocument(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiSchemaValidateFromFile")==0){
      mex_tixiSchemaValidateFromFile(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiSchemaValidateFromString")==0){
      mex_tixiSchemaValidateFromString(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiDTDValidate")==0){
      mex_tixiDTDValidate(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiGetTextElement")==0){
      mex_tixiGetTextElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiGetDoubleElement")==0){
      mex_tixiGetDoubleElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiGetIntegerElement")==0){
      mex_tixiGetIntegerElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiGetBooleanElement")==0){
      mex_tixiGetBooleanElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiUpdateTextElement")==0){
      mex_tixiUpdateTextElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiUpdateDoubleElement")==0){
      mex_tixiUpdateDoubleElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiAddTextElement")==0){
      mex_tixiAddTextElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiAddTextElementAtIndex")==0){
      mex_tixiAddTextElementAtIndex(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiRemoveElement")==0){
      mex_tixiRemoveElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiSaveDocument")==0){
      mex_tixiSaveDocument(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiSaveCompleteDocument")==0){
      mex_tixiSaveCompleteDocument(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiSaveAndRemoveDocument")==0){
      mex_tixiSaveAndRemoveDocument(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiExportDocumentAsString")==0){
      mex_tixiExportDocumentAsString(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiImportFromString")==0){
      mex_tixiImportFromString(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiGetVectorSize")==0){
      mex_tixiGetVectorSize(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiGetFloatVector")==0){
      mex_tixiGetFloatVector(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiAddFloatVector")==0){
      mex_tixiAddFloatVector(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiCreateElement")==0){
      mex_tixiCreateElement(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiCreateElementAtIndex")==0){
      mex_tixiCreateElementAtIndex(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiAddHeader")==0){
      mex_tixiAddHeader(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiAddCpacsHeader")==0){
      mex_tixiAddCpacsHeader(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiGetNamedChildrenCount")==0){
      mex_tixiGetNamedChildrenCount(nlhs, plhs, nrhs, prhs);
      return;
  }
  else if(strcmp(functionName,"tixiAddPoint")==0){
      mex_tixiAddPoint(nlhs, plhs, nrhs, prhs);
      return;
  }
  else {
      char text[255];
      snprintf(text, 250, "%s is not a valid TIXI function!\n",functionName);
      mexErrMsgTxt(text);
  }
}

#ifdef __cplusplus
}
#endif