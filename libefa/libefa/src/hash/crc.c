/* By Bob Jenkins, (c) 2006, Public Domain
   Modified by: Christian Göttel <christian.goettel@unifr.ch>  */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "crc.h"

static void build_crc_table(void) __attribute__ ((unused));

#if __WORDSIZE == 32
static const uint32_t crctab[256] = {
  0x00000000U, 0x77073096U, 0xee0e612cU, 0x990951baU, 0x076dc419U, 0x706af48fU,
  0xe963a535U, 0x9e6495a3U, 0x0edb8832U, 0x79dcb8a4U, 0xe0d5e91eU, 0x97d2d988U,
  0x09b64c2bU, 0x7eb17cbdU, 0xe7b82d07U, 0x90bf1d91U, 0x1db71064U, 0x6ab020f2U,
  0xf3b97148U, 0x84be41deU, 0x1adad47dU, 0x6ddde4ebU, 0xf4d4b551U, 0x83d385c7U,
  0x136c9856U, 0x646ba8c0U, 0xfd62f97aU, 0x8a65c9ecU, 0x14015c4fU, 0x63066cd9U,
  0xfa0f3d63U, 0x8d080df5U, 0x3b6e20c8U, 0x4c69105eU, 0xd56041e4U, 0xa2677172U,
  0x3c03e4d1U, 0x4b04d447U, 0xd20d85fdU, 0xa50ab56bU, 0x35b5a8faU, 0x42b2986cU,
  0xdbbbc9d6U, 0xacbcf940U, 0x32d86ce3U, 0x45df5c75U, 0xdcd60dcfU, 0xabd13d59U,
  0x26d930acU, 0x51de003aU, 0xc8d75180U, 0xbfd06116U, 0x21b4f4b5U, 0x56b3c423U,
  0xcfba9599U, 0xb8bda50fU, 0x2802b89eU, 0x5f058808U, 0xc60cd9b2U, 0xb10be924U,
  0x2f6f7c87U, 0x58684c11U, 0xc1611dabU, 0xb6662d3dU, 0x76dc4190U, 0x01db7106U,
  0x98d220bcU, 0xefd5102aU, 0x71b18589U, 0x06b6b51fU, 0x9fbfe4a5U, 0xe8b8d433U,
  0x7807c9a2U, 0x0f00f934U, 0x9609a88eU, 0xe10e9818U, 0x7f6a0dbbU, 0x086d3d2dU,
  0x91646c97U, 0xe6635c01U, 0x6b6b51f4U, 0x1c6c6162U, 0x856530d8U, 0xf262004eU,
  0x6c0695edU, 0x1b01a57bU, 0x8208f4c1U, 0xf50fc457U, 0x65b0d9c6U, 0x12b7e950U,
  0x8bbeb8eaU, 0xfcb9887cU, 0x62dd1ddfU, 0x15da2d49U, 0x8cd37cf3U, 0xfbd44c65U,
  0x4db26158U, 0x3ab551ceU, 0xa3bc0074U, 0xd4bb30e2U, 0x4adfa541U, 0x3dd895d7U,
  0xa4d1c46dU, 0xd3d6f4fbU, 0x4369e96aU, 0x346ed9fcU, 0xad678846U, 0xda60b8d0U,
  0x44042d73U, 0x33031de5U, 0xaa0a4c5fU, 0xdd0d7cc9U, 0x5005713cU, 0x270241aaU,
  0xbe0b1010U, 0xc90c2086U, 0x5768b525U, 0x206f85b3U, 0xb966d409U, 0xce61e49fU,
  0x5edef90eU, 0x29d9c998U, 0xb0d09822U, 0xc7d7a8b4U, 0x59b33d17U, 0x2eb40d81U,
  0xb7bd5c3bU, 0xc0ba6cadU, 0xedb88320U, 0x9abfb3b6U, 0x03b6e20cU, 0x74b1d29aU,
  0xead54739U, 0x9dd277afU, 0x04db2615U, 0x73dc1683U, 0xe3630b12U, 0x94643b84U,
  0x0d6d6a3eU, 0x7a6a5aa8U, 0xe40ecf0bU, 0x9309ff9dU, 0x0a00ae27U, 0x7d079eb1U,
  0xf00f9344U, 0x8708a3d2U, 0x1e01f268U, 0x6906c2feU, 0xf762575dU, 0x806567cbU,
  0x196c3671U, 0x6e6b06e7U, 0xfed41b76U, 0x89d32be0U, 0x10da7a5aU, 0x67dd4accU,
  0xf9b9df6fU, 0x8ebeeff9U, 0x17b7be43U, 0x60b08ed5U, 0xd6d6a3e8U, 0xa1d1937eU,
  0x38d8c2c4U, 0x4fdff252U, 0xd1bb67f1U, 0xa6bc5767U, 0x3fb506ddU, 0x48b2364bU,
  0xd80d2bdaU, 0xaf0a1b4cU, 0x36034af6U, 0x41047a60U, 0xdf60efc3U, 0xa867df55U,
  0x316e8eefU, 0x4669be79U, 0xcb61b38cU, 0xbc66831aU, 0x256fd2a0U, 0x5268e236U,
  0xcc0c7795U, 0xbb0b4703U, 0x220216b9U, 0x5505262fU, 0xc5ba3bbeU, 0xb2bd0b28U,
  0x2bb45a92U, 0x5cb36a04U, 0xc2d7ffa7U, 0xb5d0cf31U, 0x2cd99e8bU, 0x5bdeae1dU,
  0x9b64c2b0U, 0xec63f226U, 0x756aa39cU, 0x026d930aU, 0x9c0906a9U, 0xeb0e363fU,
  0x72076785U, 0x05005713U, 0x95bf4a82U, 0xe2b87a14U, 0x7bb12baeU, 0x0cb61b38U,
  0x92d28e9bU, 0xe5d5be0dU, 0x7cdcefb7U, 0x0bdbdf21U, 0x86d3d2d4U, 0xf1d4e242U,
  0x68ddb3f8U, 0x1fda836eU, 0x81be16cdU, 0xf6b9265bU, 0x6fb077e1U, 0x18b74777U,
  0x88085ae6U, 0xff0f6a70U, 0x66063bcaU, 0x11010b5cU, 0x8f659effU, 0xf862ae69U,
  0x616bffd3U, 0x166ccf45U, 0xa00ae278U, 0xd70dd2eeU, 0x4e048354U, 0x3903b3c2U,
  0xa7672661U, 0xd06016f7U, 0x4969474dU, 0x3e6e77dbU, 0xaed16a4aU, 0xd9d65adcU,
  0x40df0b66U, 0x37d83bf0U, 0xa9bcae53U, 0xdebb9ec5U, 0x47b2cf7fU, 0x30b5ffe9U,
  0xbdbdf21cU, 0xcabac28aU, 0x53b39330U, 0x24b4a3a6U, 0xbad03605U, 0xcdd70693U,
  0x54de5729U, 0x23d967bfU, 0xb3667a2eU, 0xc4614ab8U, 0x5d681b02U, 0x2a6f2b94U,
  0xb40bbe37U, 0xc30c8ea1U, 0x5a05df1bU, 0x2d02ef8dU,
};

/* how to derive the values in crctab[] from polynomial 0xedb88320 */
static void build_crc_table(void) {
  uint32_t i, j;
  for (i = 0; i < 256; i++) {
    j = i;
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    j = (j>>1) ^ ((j&1) ? 0xedb88320U : 0); 
    printf("0x%.8x, ", j);
    if (i%6 == 5) printf("\n");
  }
}
#elif __WORDSIZE == 64
static const uint64_t crctab[256] = {
  0x0000000000000000U, 0x3c3b78e888d80fe1U, 0x7876f1d111b01fc2U, 0x444d893999681023U, 0x750c207570b452a3U, 0x4937589df86c5d42U, 
  0x0d7ad1a461044d61U, 0x3141a94ce9dc4280U, 0x6ff9833db2bcc861U, 0x53c2fbd53a64c780U, 0x178f72eca30cd7a3U, 0x2bb40a042bd4d842U, 
  0x1af5a348c2089ac2U, 0x26cedba04ad09523U, 0x62835299d3b88500U, 0x5eb82a715b608ae1U, 0x5a12c5ac36adfde5U, 0x6629bd44be75f204U, 
  0x2264347d271de227U, 0x1e5f4c95afc5edc6U, 0x2f1ee5d94619af46U, 0x13259d31cec1a0a7U, 0x5768140857a9b084U, 0x6b536ce0df71bf65U, 
  0x35eb469184113584U, 0x09d03e790cc93a65U, 0x4d9db74095a12a46U, 0x71a6cfa81d7925a7U, 0x40e766e4f4a56727U, 0x7cdc1e0c7c7d68c6U, 
  0x38919735e51578e5U, 0x04aaefdd6dcd7704U, 0x31c4488f3e8f96edU, 0x0dff3067b657990cU, 0x49b2b95e2f3f892fU, 0x7589c1b6a7e786ceU, 
  0x44c868fa4e3bc44eU, 0x78f31012c6e3cbafU, 0x3cbe992b5f8bdb8cU, 0x0085e1c3d753d46dU, 0x5e3dcbb28c335e8cU, 0x6206b35a04eb516dU, 
  0x264b3a639d83414eU, 0x1a70428b155b4eafU, 0x2b31ebc7fc870c2fU, 0x170a932f745f03ceU, 0x53471a16ed3713edU, 0x6f7c62fe65ef1c0cU, 
  0x6bd68d2308226b08U, 0x57edf5cb80fa64e9U, 0x13a07cf2199274caU, 0x2f9b041a914a7b2bU, 0x1edaad56789639abU, 0x22e1d5bef04e364aU, 
  0x66ac5c8769262669U, 0x5a97246fe1fe2988U, 0x042f0e1eba9ea369U, 0x381476f63246ac88U, 0x7c59ffcfab2ebcabU, 0x4062872723f6b34aU, 
  0x71232e6bca2af1caU, 0x4d18568342f2fe2bU, 0x0955dfbadb9aee08U, 0x356ea7525342e1e9U, 0x6388911e7d1f2ddaU, 0x5fb3e9f6f5c7223bU, 
  0x1bfe60cf6caf3218U, 0x27c51827e4773df9U, 0x1684b16b0dab7f79U, 0x2abfc98385737098U, 0x6ef240ba1c1b60bbU, 0x52c9385294c36f5aU, 
  0x0c711223cfa3e5bbU, 0x304a6acb477bea5aU, 0x7407e3f2de13fa79U, 0x483c9b1a56cbf598U, 0x797d3256bf17b718U, 0x45464abe37cfb8f9U, 
  0x010bc387aea7a8daU, 0x3d30bb6f267fa73bU, 0x399a54b24bb2d03fU, 0x05a12c5ac36adfdeU, 0x41eca5635a02cffdU, 0x7dd7dd8bd2dac01cU, 
  0x4c9674c73b06829cU, 0x70ad0c2fb3de8d7dU, 0x34e085162ab69d5eU, 0x08dbfdfea26e92bfU, 0x5663d78ff90e185eU, 0x6a58af6771d617bfU, 
  0x2e15265ee8be079cU, 0x122e5eb66066087dU, 0x236ff7fa89ba4afdU, 0x1f548f120162451cU, 0x5b19062b980a553fU, 0x67227ec310d25adeU, 
  0x524cd9914390bb37U, 0x6e77a179cb48b4d6U, 0x2a3a28405220a4f5U, 0x160150a8daf8ab14U, 0x2740f9e43324e994U, 0x1b7b810cbbfce675U, 
  0x5f3608352294f656U, 0x630d70ddaa4cf9b7U, 0x3db55aacf12c7356U, 0x018e224479f47cb7U, 0x45c3ab7de09c6c94U, 0x79f8d39568446375U, 
  0x48b97ad9819821f5U, 0x7482023109402e14U, 0x30cf8b0890283e37U, 0x0cf4f3e018f031d6U, 0x085e1c3d753d46d2U, 0x346564d5fde54933U, 
  0x7028edec648d5910U, 0x4c139504ec5556f1U, 0x7d523c4805891471U, 0x416944a08d511b90U, 0x0524cd9914390bb3U, 0x391fb5719ce10452U, 
  0x67a79f00c7818eb3U, 0x5b9ce7e84f598152U, 0x1fd16ed1d6319171U, 0x23ea16395ee99e90U, 0x12abbf75b735dc10U, 0x2e90c79d3fedd3f1U, 
  0x6add4ea4a685c3d2U, 0x56e6364c2e5dcc33U, 0x42f0e1eba9ea3693U, 0x7ecb990321323972U, 0x3a86103ab85a2951U, 0x06bd68d2308226b0U, 
  0x37fcc19ed95e6430U, 0x0bc7b97651866bd1U, 0x4f8a304fc8ee7bf2U, 0x73b148a740367413U, 0x2d0962d61b56fef2U, 0x11321a3e938ef113U, 
  0x557f93070ae6e130U, 0x6944ebef823eeed1U, 0x580542a36be2ac51U, 0x643e3a4be33aa3b0U, 0x2073b3727a52b393U, 0x1c48cb9af28abc72U, 
  0x18e224479f47cb76U, 0x24d95caf179fc497U, 0x6094d5968ef7d4b4U, 0x5cafad7e062fdb55U, 0x6dee0432eff399d5U, 0x51d57cda672b9634U, 
  0x1598f5e3fe438617U, 0x29a38d0b769b89f6U, 0x771ba77a2dfb0317U, 0x4b20df92a5230cf6U, 0x0f6d56ab3c4b1cd5U, 0x33562e43b4931334U, 
  0x0217870f5d4f51b4U, 0x3e2cffe7d5975e55U, 0x7a6176de4cff4e76U, 0x465a0e36c4274197U, 0x7334a9649765a07eU, 0x4f0fd18c1fbdaf9fU, 
  0x0b4258b586d5bfbcU, 0x3779205d0e0db05dU, 0x06388911e7d1f2ddU, 0x3a03f1f96f09fd3cU, 0x7e4e78c0f661ed1fU, 0x427500287eb9e2feU, 
  0x1ccd2a5925d9681fU, 0x20f652b1ad0167feU, 0x64bbdb88346977ddU, 0x5880a360bcb1783cU, 0x69c10a2c556d3abcU, 0x55fa72c4ddb5355dU, 
  0x11b7fbfd44dd257eU, 0x2d8c8315cc052a9fU, 0x29266cc8a1c85d9bU, 0x151d14202910527aU, 0x51509d19b0784259U, 0x6d6be5f138a04db8U, 
  0x5c2a4cbdd17c0f38U, 0x6011345559a400d9U, 0x245cbd6cc0cc10faU, 0x1867c58448141f1bU, 0x46dfeff5137495faU, 0x7ae4971d9bac9a1bU, 
  0x3ea91e2402c48a38U, 0x029266cc8a1c85d9U, 0x33d3cf8063c0c759U, 0x0fe8b768eb18c8b8U, 0x4ba53e517270d89bU, 0x779e46b9faa8d77aU, 
  0x217870f5d4f51b49U, 0x1d43081d5c2d14a8U, 0x590e8124c545048bU, 0x6535f9cc4d9d0b6aU, 0x54745080a44149eaU, 0x684f28682c99460bU, 
  0x2c02a151b5f15628U, 0x1039d9b93d2959c9U, 0x4e81f3c86649d328U, 0x72ba8b20ee91dcc9U, 0x36f7021977f9cceaU, 0x0acc7af1ff21c30bU, 
  0x3b8dd3bd16fd818bU, 0x07b6ab559e258e6aU, 0x43fb226c074d9e49U, 0x7fc05a848f9591a8U, 0x7b6ab559e258e6acU, 0x4751cdb16a80e94dU, 
  0x031c4488f3e8f96eU, 0x3f273c607b30f68fU, 0x0e66952c92ecb40fU, 0x325dedc41a34bbeeU, 0x761064fd835cabcdU, 0x4a2b1c150b84a42cU, 
  0x1493366450e42ecdU, 0x28a84e8cd83c212cU, 0x6ce5c7b54154310fU, 0x50debf5dc98c3eeeU, 0x619f161120507c6eU, 0x5da46ef9a888738fU, 
  0x19e9e7c031e063acU, 0x25d29f28b9386c4dU, 0x10bc387aea7a8da4U, 0x2c87409262a28245U, 0x68cac9abfbca9266U, 0x54f1b14373129d87U, 
  0x65b0180f9acedf07U, 0x598b60e71216d0e6U, 0x1dc6e9de8b7ec0c5U, 0x21fd913603a6cf24U, 0x7f45bb4758c645c5U, 0x437ec3afd01e4a24U, 
  0x07334a9649765a07U, 0x3b08327ec1ae55e6U, 0x0a499b3228721766U, 0x3672e3daa0aa1887U, 0x723f6ae339c208a4U, 0x4e04120bb11a0745U, 
  0x4aaefdd6dcd77041U, 0x7695853e540f7fa0U, 0x32d80c07cd676f83U, 0x0ee374ef45bf6062U, 0x3fa2dda3ac6322e2U, 0x0399a54b24bb2d03U, 
  0x47d42c72bdd33d20U, 0x7bef549a350b32c1U, 0x25577eeb6e6bb820U, 0x196c0603e6b3b7c1U, 0x5d218f3a7fdba7e2U, 0x611af7d2f703a803U, 
  0x505b5e9e1edfea83U, 0x6c6026769607e562U, 0x282daf4f0f6ff541U, 0x1416d7a787b7faa0U,
};

/* how to derive the values in crctab[] from polynomial 0x42f0e1eba9ea3693 */
static void build_crc_table(void) {
  uint64_t i, j;
  for (i = 0; i < 256; i++) {
    j = i;
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    j = (j>>1) ^ ((j&1) ? 0x42f0e1eba9ea3693U : 0); 
    printf("0x%.16lxU, ", j);
    if (i%6 == 5) printf("\n");
  }
}
#endif /* BUILD_XX */

/* the hash function */
#if __WORDSIZE == 32
uint32_t crc(const char *key, unsigned int len,
	     void *foo __attribute__ ((unused)))
#elif __WORDSIZE == 64
uint64_t crc(const char *key, unsigned int len,
	     void *foo __attribute__ ((unused)))
#endif
{
  unsigned int i;

#if __WORDSIZE == 32
  uint32_t hash;
#elif __WORDSIZE == 64
  uint64_t hash;
#endif

  const unsigned char *k = (const unsigned char *)key;

  for (hash = len, i = 0; i < len; i++)
    hash = (hash >> 8) ^ crctab[(hash & 0xff) ^ k[i]];

  return hash;
}
