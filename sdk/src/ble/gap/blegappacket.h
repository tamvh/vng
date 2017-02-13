#ifndef BLEGAPPACKET_H
#define BLEGAPPACKET_H
#include <sdkdefs.h>
#include <ble/bledefs.h>

namespace iot {
namespace ble {
namespace gap {
class SDKSHARED_EXPORT Packet
{
public:
    typedef struct _Filed {
        uint8_t length;
        uint8_t type;
        uint8_t payload[0];
    } __attribute__((__packed__)) *Field;

    enum Type {
        TypeFlags                              = 0x01, /**< Flags, refer to GapAdvertisingData::Flags_t. */
        TypeIncompleteList16BitServiceIds      = 0x02, /**< Incomplete list of 16-bit Service IDs. */
        TypeCompleteList16BitServiceIds        = 0x03, /**< Complete list of 16-bit Service IDs. */
        TypeIncompleteList32BitServiceIds      = 0x04, /**< Incomplete list of 32-bit Service IDs (not relevant for Bluetooth 4.0). */
        TypeCompleteList32BitServiceIds        = 0x05, /**< Complete list of 32-bit Service IDs (not relevant for Bluetooth 4.0). */
        TypeIncompleteList128BitServiceIds     = 0x06, /**< Incomplete list of 128-bit Service IDs. */
        TypeCompleteList128BitServiceIds       = 0x07, /**< Complete list of 128-bit Service IDs. */
        TypeShortenedLocalName                 = 0x08, /**< Shortened Local Name. */
        TypeCompleteLocalName                  = 0x09, /**< Complete Local Name. */
        TypeTxPowerLevel                       = 0x0A, /**< TX Power Level (in dBm). */
        TypeDeviceId                           = 0x10, /**< Device ID. */
        TypeSlaveConnectionIntervalRange       = 0x12, /**< Slave Connection Interval Range. */
        TypeList128BitSolicitationIds          = 0x15, /**< List of 128 bit service UUIDs the device is looking for. */
        TypeServiceData                        = 0x16, /**< Service Data. */
        TypeAppearance                         = 0x19, /**< Appearance, refer to GapAdvertisingData::Appearance_t. */
        TypeAdvertisingInterval                = 0x1A, /**< Advertising Interval. */
        TypeManufacturerSpecificData           = 0xFF  /**< Manufacturer Specific Data. */
    };
    enum Flags {
        FlagsLELimitedDiscoverable = 0x01, /**< Peripheral device is discoverable for a limited period of time. */
        FlagsLEGeneralDiscoverable = 0x02, /**< Peripheral device is discoverable at any moment. */
        FlagsBredrNotDupported     = 0x04, /**< Peripheral device is LE only. */
        FlagsSimultaneousLeBredrC  = 0x08, /**< Not relevant - central mode only. */
        FlagsSimultaneousLeBredrH  = 0x10  /**< Not relevant - central mode only. */
    };

    enum Appearance_t {
        AppearanceUnknown                                   = 0,     /**< Unknown or unspecified appearance type. */
        AppearanceGenericPhone                              = 64,    /**< Generic Phone. */
        AppearanceGenericComputer                           = 128,   /**< Generic Computer. */
        AppearanceGenericWatch                              = 192,   /**< Generic Watch. */
        AppearanceWatchSportsWatch                          = 193,   /**< Sports Watch. */
        AppearanceGenericClock                              = 256,   /**< Generic Clock. */
        AppearanceGenericDisplay                            = 320,   /**< Generic Display. */
        AppearanceGenericRemoteControl                      = 384,   /**< Generic Remote Control. */
        AppearanceGenericEyeglasses                         = 448,   /**< Generic Eye Glasses. */
        AppearanceGenericTag                                = 512,   /**< Generic Tag. */
        AppearanceGenericKeyring                            = 576,   /**< Generic Keyring. */
        AppearanceGenericMediaPlayer                        = 640,   /**< Generic Media Player. */
        AppearanceGenericBarcodeScanner                     = 704,   /**< Generic Barcode Scanner. */
        AppearanceGenericThermometer                        = 768,   /**< Generic Thermometer. */
        AppearanceThermometerEar                            = 769,   /**< Ear Thermometer. */
        AppearanceGenericHeartRateSensor                    = 832,   /**< Generic Heart Rate Sensor. */
        AppearanceHeartRateSensorHeartRateBelt              = 833,   /**< Belt Heart Rate Sensor. */
        AppearanceGenericBloodPressure                      = 896,   /**< Generic Blood Pressure. */
        AppearanceBloodPressureArm                          = 897,   /**< Arm Blood Pressure. */
        AppearanceBloodPressureWrist                        = 898,   /**< Wrist Blood Pressure. */
        AppearanceHumanInterfaceDeviceHid                   = 960,   /**< Human Interface Device (HID). */
        AppearanceKeyboard                                  = 961,   /**< Keyboard. */
        AppearanceMouse                                     = 962,   /**< Mouse. */
        AppearanceJoystick                                  = 963,   /**< Joystick. */
        AppearanceGamepad                                   = 964,   /**< Gamepad. */
        AppearanceDigitizerTablet                           = 965,   /**< Digitizer Tablet. */
        AppearanceCardReader                                = 966,   /**< Card Reader. */
        AppearanceDigitalPen                                = 967,   /**< Digital Pen. */
        AppearanceBarcodeScanner                            = 968,   /**< Barcode Scanner. */
        AppearanceGenericGlucose_meter                      = 1024,  /**< Generic Glucose Meter. */
        AppearanceGenericRunningWalkingSensor               = 1088,  /**< Generic Running/Walking Sensor. */
        AppearanceRunningWalkingSensor_inShoe               = 1089,  /**< In Shoe Running/Walking Sensor. */
        AppearanceRunningWalkingSensor_onShoe               = 1090,  /**< On Shoe Running/Walking Sensor. */
        AppearanceRunningWalkingSensor_onHip                = 1091,  /**< On Hip Running/Walking Sensor. */
        AppearanceGenericCycling                            = 1152,  /**< Generic Cycling. */
        AppearanceCyclingCyclingComputer                    = 1153,  /**< Cycling Computer. */
        AppearanceCyclingSpeedSensor                        = 1154,  /**< Cycling Speed Sensor. */
        AppearanceCyclingCadenceSensor                      = 1155,  /**< Cycling Cadence Sensor. */
        AppearanceCyclingPowerSensor                        = 1156,  /**< Cycling Power Sensor. */
        AppearanceCyclingSpeedAndCadenceSensor              = 1157,  /**< Cycling Speed and Cadence Sensor. */
        AppearancePulseOximeterGeneric                      = 3136,  /**< Generic Pulse Oximeter. */
        AppearancePulseOximeter_fingertip                   = 3137,  /**< Fingertip Pulse Oximeter. */
        AppearancePulseOximeterWristWorn                    = 3138,  /**< Wrist Worn Pulse Oximeter. */
        AppearanceGenericWeightScale                        = 3200,  /**< Generic Weight Scale. */
        AppearanceOutdoorGeneric                            = 5184,  /**< Generic Outdoor. */
        AppearanceOutdoorLocationDisplayDevice              = 5185,  /**< Outdoor Location Display Device. */
        AppearanceOutdoorLocationAndNavigationDisplayDevice = 5186,  /**< Outdoor Location and Navigation Display Device. */
        AppearanceOutdoorLocationPod                        = 5187,  /**< Outdoor Location Pod. */
        AppearanceOutdoorLocationAndNavigationPod           = 5188   /**< Outdoor Location and Navigation Pod. */
    };

    Packet();

    virtual ~Packet();

    int setPayload(const Payload &payload);

    int setPayload(const uint8_t *payload, uint8_t size);

    /**
     * Adds advertising data based on the specified AD type (see GapAdvertisingData::DataType_t).
     * If the supplied AD type is already present in the advertising
     * payload, then the value is updated.
     *
     * @param[in] type The Advertising 'DataType' to add.
     * @param[in] payload Pointer to the payload contents.
     * @param[in] length Size of the payload in bytes.
     *
     * @return -2 if the new value causes the
     *         advertising buffer to overflow. 0 is returned
     *         on success.
     *
     *
     * @note When the specified AD type is TypeIncompleteList16BitServiceIds,
     *       TypeCompleteList16BitServiceIds, TypeIncompleteList32BitServiceIds,
     *       TypeCompleteList32BitServiceIds, TypeIncompleteList128BitServiceIds,
     *       TypeCompleteList128BitServiceIds or TypeList128BitSolicitationIds the
     *       supplied value is appended to the values previously added to the
     *       payload.
     */
    int setFiled(uint8_t type, const uint8_t *payload, uint8_t length);

    /**
     * Search advertisement data for a specific field.
     *
     * @param[in] type
     *              The type of the field to find.
     *
     * @return A pointer to the first element in the field if found, NULL otherwise.
     *         Where the first element is the length of the field.
     */
    Field field(uint8_t type);


    /**
     * Access the current payload.
     *
     * @return A pointer to the current payload.
     */
    const uint8_t *payload() const;

    /**
     * Get the current payload length.
     *
     * @return The current payload length (0..31 bytes).
     */
    uint8_t length() const;

private:
    class Private;
    Private *_private;
};

} // namespace gap
} // namespace ble
} // namespace iot

#endif // BLEGAPPACKET_H
