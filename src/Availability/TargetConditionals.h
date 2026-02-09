/*
 * Copyright (c) 2000-2014 by Apple Inc.. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
 
/*
     File:       TargetConditionals.h
 
     Contains:   Autoconfiguration of TARGET_ conditionals for Mac OS X and iPhone
     
                 Note:  TargetConditionals.h in 3.4 Universal Interfaces works
                        with all compilers.  This header only recognizes compilers
                        known to run on Mac OS X.
  
*/

#ifndef __TARGETCONDITIONALS__
#define __TARGETCONDITIONALS__

// Make sure all TARGET_OS_* and TARGET_CPU_* values are defined
#ifndef TARGET_OS_MAC
    #define TARGET_OS_MAC        0
#endif

#ifndef TARGET_OS_OSX
    #define TARGET_OS_OSX        0
#endif

#ifndef TARGET_OS_IPHONE
    #define TARGET_OS_IPHONE     0
#endif

#ifndef TARGET_OS_IOS
    #define TARGET_OS_IOS        0
#endif

#ifndef TARGET_OS_WATCH
    #define TARGET_OS_WATCH      0
#endif

#ifndef TARGET_OS_TV
    #define TARGET_OS_TV         0
#endif

#ifndef TARGET_OS_SIMULATOR
    #define TARGET_OS_SIMULATOR  0
#endif

#ifndef TARGET_OS_EMBEDDED
    #define TARGET_OS_EMBEDDED   0
#endif

#ifndef TARGET_OS_RTKIT
    #define TARGET_OS_RTKIT      0
#endif

#ifndef TARGET_OS_MACCATALYST
    #define TARGET_OS_MACCATALYST 0
#endif

#ifndef TARGET_OS_VISION
    #define TARGET_OS_VISION     0
#endif

#ifndef TARGET_OS_UIKITFORMAC
    #define TARGET_OS_UIKITFORMAC 0
#endif

#ifndef TARGET_OS_DRIVERKIT
    #define TARGET_OS_DRIVERKIT 0
#endif 

#ifndef TARGET_OS_WIN32
    #define TARGET_OS_WIN32     0
#endif

#ifndef TARGET_OS_WINDOWS
    #define TARGET_OS_WINDOWS   0
#endif



#ifndef TARGET_OS_LINUX
    #define TARGET_OS_LINUX     0
#endif

#ifndef TARGET_CPU_PPC
    #define TARGET_CPU_PPC      0
#endif

#ifndef TARGET_CPU_PPC64
    #define TARGET_CPU_PPC64    0
#endif

#ifndef TARGET_CPU_68K
    #define TARGET_CPU_68K      0
#endif

#ifndef TARGET_CPU_X86
    #define TARGET_CPU_X86      0
#endif

#ifndef TARGET_CPU_X86_64
    #define TARGET_CPU_X86_64   0
#endif

#ifndef TARGET_CPU_ARM
    #define TARGET_CPU_ARM      0
#endif

#ifndef TARGET_CPU_ARM64
    #define TARGET_CPU_ARM64    0
#endif

#ifndef TARGET_CPU_MIPS
    #define TARGET_CPU_MIPS     0
#endif

#ifndef TARGET_CPU_SPARC
    #define TARGET_CPU_SPARC    0
#endif

#ifndef TARGET_CPU_ALPHA
    #define TARGET_CPU_ALPHA    0
#endif

#ifndef TARGET_ABI_USES_IOS_VALUES
    #define TARGET_ABI_USES_IOS_VALUES  (!TARGET_CPU_X86_64 || (TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST))
#endif

#ifndef TARGET_IPHONE_SIMULATOR
    #define TARGET_IPHONE_SIMULATOR     TARGET_OS_SIMULATOR /* deprecated */
#endif

#ifndef TARGET_OS_NANO
    #define TARGET_OS_NANO              TARGET_OS_WATCH /* deprecated */
#endif

#endif

