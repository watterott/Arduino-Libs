
typedef struct __attribute__((packed))
{
  uint8_t  magic[2];
  uint32_t size;
  uint16_t rsrvd1;
  uint16_t rsrvd2;
  uint32_t offset;
} BMP_Header;


typedef struct __attribute__((packed))
{
  uint32_t size;
  uint32_t width;
  uint32_t height;
  uint16_t nplanes;
  uint16_t bitspp;
  uint32_t compress;
  uint32_t isize;
  uint32_t hres;
  uint32_t vres;
  uint32_t colors;
  uint32_t impcolors;
} BMP_DIPHeader;
