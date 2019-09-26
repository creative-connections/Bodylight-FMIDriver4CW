#define vtNothing          0   // indicates "not having been used before"
#define vtError            1   // error tag
#define vtBoolean          2   // 8 bit flag
#define vtShortCard        3   // 8 bit unsigned
#define vtCardinal         4   // 16 bit unsigned
#define vtLongCard         5   // 32 bit unsigned
#define vtShortInt         6   // 8 bit signed
#define vtInteger          7   // 16 bit signed
#define vtLongInt          8   // 32 bit signed
#define vtReal             9   // float  (dword)
#define vtLongReal         10  // double (qword)
#define vtPString          11  // char (* pString)[255]
#define vtBuffer           12  // buffer
#define vtUnknown          13  // unknown value
#define vtDriverString     15  // driver string

// channel errors (ce)
#define ceOK              0
#define ceTimeout         1     // timout
#define ceValue           2     // invalid value
#define ceUnknownChannel  3     // unknown channel

// driver callback function (dcf)
#define dcfDriverInputFinalized     0   
#define dcfDriverOutputFinalized    1
#define dcfDriverInterrupt          2
#define dcfOOBDataAdvice            3

// buffer Header Kinds
#define hkStdBuffer        0   // standard buffer format (header+data block)

// buffer status - bits        // be aware - these value represents bits, not masks
#define stEmpty            0   // empty data
#define stFillInProgress   1   // filling buffer
#define stFull             2   // buffer completely valid
#define stError            3   // not completely valid or if something bad occured
#define stBusy             4   // if setting channel or something calculating
#define stInvalid          5   // 2nd phase after stNonactual (before channels read)
#define stNonactual        6   // if exists new (not actually read) data
#define stModified         15  // used by instrument to mark modification;
// thus can be set channel only if neccessary

typedef char tString255[255];
typedef unsigned(*tDriverCallback) (HANDLE, unsigned, void *);
typedef unsigned channelNumber;

typedef struct tBufferHeader { // Buffer header
  struct {
    unsigned short HeaderKind;
    union {
      struct {              // in case of HeaderKind == hkStdBuffer
        unsigned short Channels;     // number of channels
        unsigned       NumSamples;   // volume of data block
        unsigned       CurSamples;   // number of samples currently present
        double         SampFreq;     // sampling frequency or 0 (0 indicates "Time" usage)
        unsigned short SampStruct;   // sample structure (0==real data, 1==complex & Channels==2)
        unsigned short DataType;     // type of data (valueXXX)
        unsigned short Status;       // buffer status (stXXX)
        unsigned short Any1;         // not used now; leave 0
        float          Time;         // sampling time, not period
      };
    };
  };
} tBufferHeader;                                // size of tBufferHeader must be 32 bytes

#pragma pack(push, 4) 
struct TValue {  // Value storage
  struct {
    unsigned Type;
    union {                                           // switch (Type)
      char               ValBoolean;                // vtBoolean
      unsigned char      ValShortCard;              // vtShortCard
      unsigned short     ValCardinal;               // vtCardinal
      unsigned           ValLongCard;               // vtLongCard
      signed char        ValShortInt;               // vtShortInt
      signed short       ValInteger;                // vtInteger
      signed             ValLongInt;                // vtLongInt
      float              ValReal;                   // vtReal
      double             ValLongReal;               // vtLongReal
      tString255         *ValPString;               // vtPString
      struct {                                      // vtDriverString
        unsigned       StringCharLength;
        void           *PString;
      };
      struct {                                      // vtBuffer
        unsigned char  BType;
        unsigned short BLen256;
        void           *PBuffer;
      };
    };
  };
};
#pragma pack(pop)


typedef struct timeStamp {
  // data time stamp
  unsigned       MillisecondsInDay;
  unsigned char  Day;
  unsigned char  Month;
  unsigned short Year;
} timeStamp;
