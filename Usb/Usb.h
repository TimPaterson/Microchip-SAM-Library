#pragma once
#pragma pack(push, 1)

//****************************************************************************
// USB Version definition
//****************************************************************************

#define USB_VER_1p1		0x0110
#define USB_VER_2p0		0x0200

//****************************************************************************
// Standard control transfers
//****************************************************************************

enum UsbStdReq
{
	USBREQ_Get_Status,
	USBREQ_Clear_Feature,
	USBREQ_Set_Feature = 3,
	USBREQ_Set_Address = 5,
	USBREQ_Get_Descriptor,
	USBREQ_Set_Descriptor,
	USBREQ_Get_Configuration,
	USBREQ_Set_Configuration,
	USBREQ_Get_Interface,
	USBREQ_Set_Interface,
	USBREQ_Synch_Frame
};

struct UsbSetupPacket
{
	union
	{
		byte	bmRequestType;
		struct  
		{
			byte	Recipient:5;
			byte	Type:2;
			byte	Dir:1;
		};
	};
	byte	bRequest;
	union
	{
		ushort	wValue;
		byte	bValue;
		struct  
		{
			byte	bDescIndex;
			byte	bDescType;
		};
	};
	ushort	wIndex;
	ushort	wLength;
};

// Bits in bmRequestType
#define	USBRT_Dir_Pos			7		// bit position
#define	USBRT_Dir_Msk			(1 << USBRT_Dir_Pos)
#define USBRT_DirOut_Val		0
#define USBRT_DirIn_Val			1
#define USBRT_DirOut			(USBRT_DirOut_Val << USBRT_Dir_Pos)
#define USBRT_DirIn				(USBRT_DirIn_Val << USBRT_Dir_Pos)

#define USBRT_Type_Pos			5		// bit position
#define USBRT_Type_Msk			(3 << USBRT_Type_Pos)
#define USBRT_TypeStd_Val		0
#define USBRT_TypeClass_Val		1
#define USBRT_TypeVendor_Val	2
#define USBRT_TypeStd			(USBRT_TypeStd_Val << USBRT_Type_Pos)
#define USBRT_TypeClass			(USBRT_TypeClass_Val << USBRT_Type_Pos)
#define USBRT_TypeVendor		(USBRT_TypeVendor_Val << USBRT_Type_Pos)

#define USBRT_Recip_Pos			0		// bit position
#define USBRT_Recip_Msk			(0x1F << USBRT_Recip_Pos)
#define USBRT_RecipDevice_Val	0
#define USBRT_RecipIface_Val	1
#define USBRT_RecipEp_Val		2
#define USBRT_RecipOther_Val	3
#define USBRT_RecipDevice		(USBRT_RecipDevice_Val << USBRT_Recip_Pos)
#define USBRT_RecipIface		(USBRT_RecipIface_Val << USBRT_Recip_Pos)
#define USBRT_RecipEp			(USBRT_RecipEp_Val << USBRT_Recip_Pos)
#define USBRT_RecipOther		(USBRT_RecipOther_Val << USBRT_Recip_Pos)

// Features for Set/Get Feature
#define USBFEAT_EndpointHalt		0
#define USBFEAT_DeviceRemoteWakeup	1
#define USBFEAT_TestMode			2

//****************************************************************************
// Structure definitions for USB descriptors
//****************************************************************************

enum UsbDescType
{
	USBDESC_Device = 1,
	USBDESC_Config,
	USBDESC_String,
	USBDESC_Interface,
	USBDESC_Endpoint,
	USBDESC_Device_qualifier,
	USBDESC_OtherSpeedConfig,
	USBDESC_InterfacePower,
	USBDESC_Otg,
	USBDESC_Debug,
	USBDESC_InterfaceAssociation
};


//****************************************************************************
// Device Descriptor
//

struct UsbDeviceDesc
{
	byte	bLength;
	byte	bDescType;
	ushort	bcdUSB;
	byte	bDeviceClass;
	byte	bDeviceSubclass;
	byte	bDeviceProtocol;
	byte	bMaxPacketSize0;
	ushort	idVendor;
	ushort	idProduct;
	ushort	bcdDevice;
	byte	iManufacturer;
	byte	iProduct;
	byte	iSerialNumber;
	byte	bNumConfigs;
};

// Device classes
#define USBDEVCLASS_None			0
#define USBDEVCLASS_Communications	2
#define USBDEVCLASS_Hub				9

//****************************************************************************
// Configuration Descriptor
//

struct UsbConfigDesc
{
	byte	bLength;
	byte	bDescType;
	ushort	wTotalLength;
	byte	bNumInterfaces;
	byte	bConfigValue;
	byte	iConfig;
	byte	bmAttributes;
	byte	bMaxPower;
};

// Definition of bmAttributes
#define USBCFG_Power_Pos			6
#define USBCFG_Power_Msk			(1 << USBCFG_Power_Pos)
#define USBCFG_PowerBus_Val			0
#define USBCFG_PowerSelf_Val		1
#define USBCFG_PowerBus				(USBCFG_PowerBus_Val << USBCFG_Power_Pos)
#define USBCFG_PowerSelf			(USBCFG_PowerBus_Val << USBCFG_Power_Pos)
#define USBCFG_RemoteWakeup_Pos		5
#define USBCFG_RemoteWakeup_Msk		(1 << USBCFG_RemoteWakeup_Msk)
#define USBCFG_RemoteWakeup_Val		1
#define USBCFG_RemoteWakeup			(USBCFG_RemoteWakeup_Val << USBCFG_RemoteWakeup_Pos)
#define USBCFG_BaseAttributes		0x80

//****************************************************************************
// String Descriptor
//

struct UsbStringDesc
{
	byte	bLength;
	byte	bDescType;
};

//****************************************************************************
// Interface Descriptor
//

struct UsbInterfaceDesc
{
	byte	bLength;
	byte	bDescType;
	byte	bInterfaceNumber;
	byte	bAlternateSetting;
	byte	bNumEndpoints;
	byte	bInterfaceClass;
	byte	bInterfaceSubclass;
	byte	bInterfaceProtocol;
	byte	iInterface;
};

// Interface classes
enum UsbClass
{
	USBCLASS_Audio = 1,
	USBCLASS_Cdc,
	USBCLASS_Hid,
	USBCLASS_Physical = 5,
	USBCLASS_Image,
	USBCLASS_Printer,
	USBCLASS_MassStorage,
	USBCLASS_Hub,
	USBCLASS_CdcData,
	USBCLASS_SmartCard,
	USBCLASS_Security = 13,
	USBCLASS_Video
};

//****************************************************************************
// Endpoint Descriptor
//

struct UsbEndpointDesc
{
	byte	bLength;
	byte	bDescType;
	byte	bEndpointAddr;
	byte	bmAttributes;
	ushort	wMaxPacketSize;
	byte	bInterval;
};

// Definitions for bEndpointAddr
#define USBEP_Dir_Pos		7	// bit position
#define USBEP_Dir_Msk		(1 << USBEP_Dir_Pos)
#define	USBEP_DirIn_Val		1
#define USBEP_DirOut_Val	0
#define USBEP_DirIn			(USBEP_DirIn_Val << USBEP_Dir_Pos)
#define USBEP_DirOut		(USBEP_DirOut_Val << USBEP_Dir_Pos)

// Definition of bmAttributes
#define USBEP_Control	0
#define USBEP_Bulk		2
#define	USBEP_Interrupt	3

#pragma pack(pop)
