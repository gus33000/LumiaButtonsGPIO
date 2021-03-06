// Copyright (c) Microsoft Corporation. All Rights Reserved. 
// Copyright (c) Bingxing Wang. All Rights Reserved. 

#pragma once

#ifndef __HID_COMMON_H__
#define __HID_COMMON_H__

#define USAGE_PAGE 0x05
#define USAGE_PAGE_1 0x06
#define USAGE      0x09
#define USAGE_MINIMUM 0x19
#define USAGE_MAXIMUM 0x29
#define LOGICAL_MINIMUM 0x15
#define LOGICAL_MAXIMUM 0x25
#define LOGICAL_MAXIMUM_2 0x26
#define LOGICAL_MAXIMUM_3 0x27
#define PHYSICAL_MINIMUM 0x35
#define PHYSICAL_MAXIMUM 0x45
#define PHYSICAL_MAXIMUM_2 0x46
#define PHYSICAL_MAXIMUM_3 0x47
#define UNIT_EXPONENT 0x55
#define UNIT 0x65
#define UNIT_2 0x66

#define REPORT_ID       0x85
#define REPORT_COUNT    0x95
#define REPORT_COUNT_2	0x96
#define REPORT_SIZE     0x75
#define INPUT           0x81
#define FEATURE         0xb1

#define BEGIN_COLLECTION 0xa1
#define END_COLLECTION   0xc0

#define REPORT_BUFFER_SIZE   1024
#define DEVICE_VERSION 0x01
#define MAX_FINGERS	16

#endif