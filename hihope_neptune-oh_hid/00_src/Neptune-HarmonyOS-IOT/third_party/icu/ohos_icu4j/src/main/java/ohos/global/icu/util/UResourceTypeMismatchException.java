/* GENERATED SOURCE. DO NOT MODIFY. */
// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
/*
******************************************************************************
* Copyright (C) 2004-2006, International Business Machines Corporation and   *
* others. All Rights Reserved.                                               *
******************************************************************************
*/

package ohos.global.icu.util;

/**
 * Exception thrown when the requested resource type 
 * is not the same type as the available resource
 * @author ram
 * @hide exposed on OHOS
 */
public class UResourceTypeMismatchException extends RuntimeException {
    // Generated by serialver from JDK 1.4.1_01
    static final long serialVersionUID = 1286569061095434541L;
    
    /**
     * Constuct the exception with the given message
     * @param msg the error message for this exception
     */
    public UResourceTypeMismatchException(String msg){
        super(msg);       
    }
}
