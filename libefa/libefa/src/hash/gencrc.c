/* By Bob Jenkins, (c) 2006, Public Domain
   Modified by: Christian Göttel <christian.goettel@unifr.ch> */

#include "gencrc.h"

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

static void build_gencrc_table(void) __attribute__ ((unused));

#if __WORDSIZE == 32
/* every byte is an arbitrary permutation of 0..255 */
static const uint32_t gencrctab[256] = {
  0x46d1e192U, 0x66edf9aaU, 0x927fc9e5U, 0xa53baaccU, 0x29b47658U, 0x5a411a01U,
  0x0e66d5bdU, 0x0dd5b1dbU, 0xcb38340eU, 0x04d4ebb6U, 0x98bc4f54U, 0x36f20f2cU,
  0x4a3047edU, 0x1ec1e0ebU, 0x568c0c1fU, 0x6a731432U, 0x81367fc6U, 0xe3e25237U,
  0xe7f64884U, 0x0fa59f64U, 0x4f3109deU, 0xf02d61f5U, 0x5daec03bU, 0x7f740e83U,
  0x056ff2d8U, 0x2026cc0aU, 0x7ac2112dU, 0x82c55605U, 0xb0911ef2U, 0xa7b88e4cU,
  0x89dca282U, 0x4b254d27U, 0x7694a6d3U, 0xd229eaddU, 0x8e8f3738U, 0x5bee7a55U,
  0x012eb6abU, 0x08dd28c8U, 0xb5abc274U, 0xbc7931f0U, 0xf2396ed5U, 0xe4e43d97U,
  0x943f4b7fU, 0x85d0293dU, 0xaed83a88U, 0xc8f932fcU, 0xc5496f20U, 0xe9228173U,
  0x9b465b7dU, 0xfda26680U, 0x1ddeab35U, 0x0c4f25cbU, 0x86e32fafU, 0xe59fa13aU,
  0xe192e2c4U, 0xf147da1aU, 0x67620a8dU, 0x5c9a24c5U, 0xfe6afde2U, 0xacad0250U,
  0xd359730bU, 0xf35203b3U, 0x96a4b44dU, 0xfbcacea6U, 0x41a165ecU, 0xd71e53acU,
  0x835f39bfU, 0x6b6bde7eU, 0xd07085baU, 0x79064e07U, 0xee5b20c3U, 0x3b90bd65U,
  0x5827aef4U, 0x4d12d31cU, 0x9143496eU, 0x6c485976U, 0xd9552733U, 0x220f6895U,
  0xe69def19U, 0xeb89cd70U, 0xc9bb9644U, 0x93ec7e0dU, 0x2ace3842U, 0x2b6158daU,
  0x039e9178U, 0xbb5367d7U, 0x55682285U, 0x4315d891U, 0x19fd8906U, 0x7d8d4448U,
  0xb4168a03U, 0x40b56a53U, 0xaa3e69e0U, 0xa25182feU, 0xad34d16cU, 0x720c4171U,
  0x9dc3b961U, 0x321db563U, 0x8b801b9eU, 0xf5971893U, 0x14cc1251U, 0x8f4ae962U,
  0xf65aff1eU, 0x13bd9deeU, 0x5e7c78c7U, 0xddb61731U, 0x73832c15U, 0xefebdd5bU,
  0x1f959acaU, 0xe801fb22U, 0xa89826ceU, 0x30b7165dU, 0x458a4077U, 0x24fec52aU,
  0x849b065fU, 0x3c6930cdU, 0xa199a81dU, 0xdb768f30U, 0x2e45c64aU, 0xff2f0d94U,
  0x4ea97917U, 0x6f572acfU, 0x653a195cU, 0x17a88c5aU, 0x27e11fb5U, 0x3f09c4c1U,
  0x2f87e71bU, 0xea1493e4U, 0xd4b3a55eU, 0xbe6090beU, 0xaf6cd9d9U, 0xda58ca00U,
  0x612b7034U, 0x31711dadU, 0x6d7db041U, 0x8ca786b7U, 0x09e8bf7aU, 0xc3c4d7eaU,
  0xa3cd77a8U, 0x7700f608U, 0xdf3de559U, 0x71c9353fU, 0x9fd236fbU, 0x1675d43eU,
  0x390d9e9aU, 0x21ba4c6bU, 0xbd1371e8U, 0x90338440U, 0xd5f163d2U, 0xb140fef9U,
  0x52f50b57U, 0x3710cf67U, 0x4c11a79cU, 0xc6d6624eU, 0x3dc7afa9U, 0x34a69969U,
  0x70544a26U, 0xf7d9ec98U, 0x7c027496U, 0x1bfb3ba3U, 0xb3b1dc8fU, 0x9a241039U,
  0xf993f5a4U, 0x15786b99U, 0x26e704f7U, 0x51503c04U, 0x028bb3b8U, 0xede5600cU,
  0x9cb22b29U, 0xb6ff339bU, 0x7e771c43U, 0xc71c05f1U, 0x604ca924U, 0x695eed60U,
  0x688ed0bcU, 0x3e0b232fU, 0xf8a39c11U, 0xbae6e67cU, 0xb8cf75e1U, 0x970321a7U,
  0x5328922bU, 0xdef3df2eU, 0x8d0443b0U, 0x2885e3aeU, 0x6435eed1U, 0xcc375e81U,
  0xa98495f6U, 0xe0bff114U, 0xb2da3e4fU, 0xc01b5adfU, 0x507e0721U, 0x6267a36aU,
  0x181a6df8U, 0x7baff0c0U, 0xfa6d6c13U, 0x427250b2U, 0xe2f742d6U, 0xcd5cc723U,
  0x2d218be7U, 0xb91fbbb1U, 0x9eb946d0U, 0x1c180810U, 0xfc81d602U, 0x0b9c3f52U,
  0xc2ea456fU, 0x1165b2c9U, 0xabf4ad75U, 0x0a56fc8cU, 0x12e0f818U, 0xcadbcba1U,
  0x2586be56U, 0x952c9b46U, 0x07c6a43cU, 0x78967df3U, 0x477b2e49U, 0x2c5d7b6dU,
  0x8a637272U, 0x59acbcb4U, 0x74a0e447U, 0xc1f8800fU, 0x35c015dcU, 0x230794c2U,
  0x4405f328U, 0xec2adba5U, 0xd832b845U, 0x6e4ed287U, 0x48e9f7a2U, 0xa44be89fU,
  0x38cbb725U, 0xbf6ef4e6U, 0xdc0e83faU, 0x54238d12U, 0xf4f0c1e3U, 0xa60857fdU,
  0xc43c64b9U, 0x00c851efU, 0x33d75f36U, 0x5fd39866U, 0xd1efa08aU, 0xa0640089U,
  0x877a978bU, 0x99175d86U, 0x57dfacbbU, 0xceb02de9U, 0xcf4d5c09U, 0x3a8813d4U,
  0xb7448816U, 0x63fa5568U, 0x06be014bU, 0xd642fa7bU, 0x10aa7c90U, 0x8082c88eU,
  0x1afcba79U, 0x7519549dU, 0x490a87ffU, 0x8820c3a0U,
};

/* how the values of gencrc[] were derived */
static void build_gencrc_table(void) {
  uint32_t tab[256];
  uint32_t i;

  /* fill tab[] with random permutations of 0..255 in each byte */
  for (i = 0; i < 256; i++) {
    unsigned int j;
    unsigned char x = (unsigned char) i;
    for (j = 0; j < 5; ++j) {x+=1; x += x<<1; x ^= x>>1;}
    tab[i] = (uint32_t) x;
    for (j = 0; j < 5; ++j) {x+=2; x += x<<1; x ^= x>>1;}
    tab[i] ^= (((uint32_t) x) << 8);
    for (j = 0; j < 5; ++j) {x+=3; x += x<<1; x ^= x>>1;}
    tab[i] ^= (((uint32_t) x) << 16);
    for (j = 0; j < 5; ++j) {x+=4; x += x<<1; x ^= x>>1;}
    tab[i] ^= (((uint32_t) x) << 24);
  }

  /* verify that 0..255 appears in each byte exactly once */
  for (i = 0; i < 256; i++) {
    unsigned int count, j;
    for (count = 0, j = 0; j < 256; ++j) if ((tab[j] & 0xffU) == i) ++count;
    if (count != 1) printf("bad1!  %d %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 8) & 0xffU) == i) ++count;
    if (count != 1) printf("bad2!  %d %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 16) & 0xffU) == i) ++count;
    if (count != 1) printf("bad3!  %d %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 24) & 0xffU) == i) ++count;
    if (count != 1) printf("bad4!  %d %d\n", i, count);
  }

  /* print out the table */
  for (i = 0; i < 256; ++i) {
    printf("0x%.8x, ", tab[i]);
    if (i%6 == 5) printf("\n");
  }
}

/* the actual hash function */
uint32_t gencrc(const char *key, unsigned int len,
		void *foo __attribute__ ((unused))) {
  uint32_t hash;
  unsigned int i;
  const unsigned char *k = (const unsigned char *)key;

  for (hash = (uint32_t) len, i = 0; i < len; i++)
    hash = (hash >> 8) ^ gencrctab[(hash & 0xffU) ^ k[i]];

  return hash;
}

#elif __WORDSIZE == 64

/* every byte is an arbitrary permutation of 0..255 */
static const uint64_t gencrctab[256] = {
  0x3ad074fb2969b18fU, 0x5662be80370f587eU, 0xae8580450fe328e8U, 0xe6ef01e3dd0de26dU, 0xd27c2eace4e4e4abU, 0x29a4afff7285dd43U, 
  0x6bd2479c063a0786U, 0x8474073caa3b43adU, 0x2593140b434efe06U, 0xb89a382c733d3df4U, 0xb00b17069cd7ee6fU, 0x6175aaa4c907864fU, 
  0x0a8a48359434fa6bU, 0xf2ec612e083997e6U, 0xcee4452b52b917bdU, 0x510f4621591a6b61U, 0xfc493f8c2c1d643dU, 0x180e27555a985bb0U, 
  0x2e27a0c43e90900cU, 0xb6c7490e39cef7caU, 0xff8cebaa7aaf6a0fU, 0x9c005b611c771487U, 0x3c254afc8294be1fU, 0xf802ec68c18f32ffU, 
  0x8f05666658f87773U, 0x03f33176ad8138ccU, 0xcf82ae8f28420c37U, 0xe73e70cfbbc637aaU, 0x50639495ed9d9d45U, 0x8b60a6d5343656b5U, 
  0x8388bbe193cfc7daU, 0x321f35c02699fbdfU, 0x282a09c7fff17028U, 0xddf9c8983573a380U, 0x5f7dd8965b135051U, 0xf3cb9a63a330dcddU, 
  0x0c6154d1622bd48aU, 0x9638831ffac28134U, 0x196c3dbd6e0496b1U, 0xfb31c2896ffd48a5U, 0xc1f6df25e7174735U, 0xa8517dbac8a76216U, 
  0xf1e8c5ee9a05f5e7U, 0xbb30f57823713c9eU, 0xd935633f6ca45e70U, 0x33f4bffe6adf231aU, 0x622bfb946dea76deU, 0xc2e91d379fc43582U, 
  0x5c2d77dff4da662aU, 0x687bb24260202e78U, 0x37b3d6585e1b0563U, 0xf0f802a9bd62d120U, 0x1223322ff6a83b66U, 0x2fc8a7f08572e393U, 
  0xcd2285ad02cae988U, 0x914beecea9b39511U, 0xa9a2cefa9e5dda0dU, 0xacb73952a5e21bbeU, 0xdb19ba18cc22ce40U, 0x0e67a199fbd34627U, 
  0xfeea9b2221e127bcU, 0xd71c0b2354317f96U, 0x9418c0dc36ec11a1U, 0x7d0886560ae8b2efU, 0x1d56a8ed14e9a80bU, 0x9371810d6b6b0f67U, 
  0x5272de17b9ae6cc3U, 0x46dc671210a59a05U, 0xc547b141a6f2618bU, 0xc7d17c36dbac8d85U, 0xde36646ccee0cdf3U, 0x88f098abd31809c0U, 
  0xd56625cae07dd859U, 0x55d610084228f8e0U, 0x7a767553d8525ad8U, 0xb5ab562074244dd0U, 0x0fb524b0d1ab222eU, 0xa36930e8d5910339U, 
  0xeda72262877b3e03U, 0xcb3dd9d6fd801c1dU, 0xc66a0313d4105cf0U, 0x609523647b78f2e5U, 0xe4defa7c4a29b446U, 0xe1df0ebb01b6307dU, 
  0x3878f74e984c4e5dU, 0x41cf8fb33fa601c2U, 0xbff78e5a83c7b542U, 0xaac46bd807c9ba6eU, 0x650133a3d768caf1U, 0xfa57692af22a5f8cU, 
  0x431e188177562cd7U, 0x156dd5839d74db30U, 0x5e650c34bf8df1d2U, 0x6dfc7685bc11d68dU, 0x445d2df9aef30b83U, 0x69c15059969b8a10U, 
  0x10e3d1a20b33ac15U, 0x636b295bf84a99f5U, 0x672c2b4c4475af4bU, 0x1a81da8d032c16d4U, 0xbcdb8d8715a9d202U, 0x59ffef9a0908c4f8U, 
  0x5ad306d905199453U, 0x8e121e4b17be8750U, 0x89454200cf638549U, 0x957e9026c083b754U, 0x13aee19e4dfa41e1U, 0xaf97b3385049b075U, 
  0xd8dd05c32f47a200U, 0x6afd6f28b45e8e64U, 0x4e2ec148be887272U, 0x31897bf58a871f38U, 0xf94fc34d51c5834cU, 0xa4c68a4a8e9f24c6U, 
  0x2b64c60f65f61ec9U, 0x8a8f0af669c0d9d9U, 0xbaa32a46d6538b7cU, 0x264c4ef13a5af371U, 0xa1da288e86ebc118U, 0x30737f9f55edd033U, 
  0x8d80520781933ffeU, 0x4a4d92440eba06fcU, 0x499f3cb433551a94U, 0xca5cd4bcf9b84555U, 0xa734abc29b4074bbU, 0x1fe237bf38eea97fU, 
  0x549bb81c3d502f01U, 0x9b0d4cf8b88a0212U, 0x47fb65b625efef17U, 0xb11d1c3a2d276d69U, 0xc4b9e5f4cdb7f447U, 0x581778f2e92513a0U, 
  0x0bee217d66e6e6edU, 0x0dfea2091367b3bfU, 0x7754ea572a12b997U, 0x1bbd4fcdc6bbc098U, 0x9096b6923154a677U, 0x08099c047cfb049aU, 
  0x0959db91d96612baU, 0xea948c8288417858U, 0xe89c726bba9280cfU, 0x71ba9511916e18ebU, 0x2226680a57a2ccfbU, 0x7e53f115c53fa1d1U, 
  0x7346f61ecb5b7b99U, 0x7f83166e7965cfceU, 0x74b408f7a4465d3eU, 0xecf2cb32e8dde03cU, 0x017a11ae7d149ccbU, 0xd0b0442484b198c5U, 
  0x146e58a68fd2c54aU, 0x1cd7d7be126dde3bU, 0x7c701314167c6f68U, 0x87037173a179365aU, 0x7b0cca5c8bdb394eU, 0x347900c56138420aU, 
  0x275b0fb5b3aa49c7U, 0x6677439be5c3b6f7U, 0x020726a818009176U, 0xe0d4f2a7f39e52f9U, 0x978bad6d676154b8U, 0xa0c01fe699064081U, 
  0xb4b65fb7008caa36U, 0x2ded12e0ebf0fd7bU, 0xda06e8887e0ae1c4U, 0x3baf15754715ec24U, 0x16c53ee7a2828289U, 0xe524198b2effdf91U, 
  0xc8289d9d8dccc3f6U, 0xabbb51b24909c919U, 0x79337e720cd0a41bU, 0x9eb85a8ab6d5d3a8U, 0x78cdd0ccf70ecbeeU, 0xf64e1a74d23c0d6aU, 
  0x4c4ac96575022a26U, 0xfd52b71ae67ffc1eU, 0x4041fe865f1c93b6U, 0xf53fc4193bf78cf2U, 0x201b93a1b716adfdU, 0x36b1ff31925c1d57U, 
  0x1721b96720a11062U, 0x065f880546b50014U, 0x3f2f0493e3895909U, 0x6ead55a05dde689cU, 0x4b988297af9615eaU, 0xe98ee62d7884bc9bU, 
  0x35d99e10c3a0a008U, 0xa6eb202941bc44dbU, 0x9ffa36d332f54b74U, 0x42aaf0e5f1437d41U, 0x57912fd2892d1931U, 0x0442c784de03ffb2U, 
  0x45a8575ec77ea725U, 0x728640e4f51faee4U, 0x2c15995f0db03a6cU, 0x5d68b41beff408e9U, 0x21d5dd030437b87aU, 0xa5cccd601b5f7a21U, 
  0xe27f4bcb97954fb3U, 0x8cbfedd49532ea1cU, 0x113bf4c8eebdbdafU, 0x3dc3a3c6400be82bU, 0x9239dc6aec4d7192U, 0x6416b5c9fe57d7d3U, 
  0xebe7acb122860a22U, 0x3992e30cdc2133b4U, 0xd35a5dec8076f95fU, 0x05135902df515179U, 0x2384bc43b13ed52fU, 0x48d8a5393c354cb9U, 
  0xcc3c1b51a8606052U, 0xc9cad3de637a79d5U, 0x7687e03efcf929aeU, 0xd1e0a4fd246f2d3aU, 0xb95ef8c168e5e55cU, 0xdff16d70704f8813U, 
  0x85900d7e27a3a5cdU, 0xa2a979491d973165U, 0x00115c7a452f92c1U, 0x2a323a27d00c7c60U, 0xd4047ad711e7bbdcU, 0x75c934a5198b845eU, 
  0x3e5873eadac18932U, 0x6f43877f90d9c2acU, 0x9d20cfe2c4649e56U, 0x811a97797f9a259dU, 0xc3a66adb4ed4f0ecU, 0x07e6e7dd4c262095U, 
  0x9a14f901e2dc2684U, 0xf4acf3afb0bf2be3U, 0x1e9d5eb81ac8c89fU, 0x6ca1964f4bd155a7U, 0xeec26cebacfc9b4dU, 0xb7a0fd16c26caba2U, 
  0x4fe1cc5d484b65a9U, 0xbe5062ef76ad57faU, 0xb299fcd0ea48532cU, 0x5310e4f3a7458f5bU, 0xdc6f843b1e8ec644U, 0xef9e4d3d5601e790U, 
  0xbdbe41334f70eb04U, 0xe3293b54b59c9f29U, 0x99b26e697123bf07U, 0x700a5340abb275e2U, 0x5bce9fe9f0584a48U, 0x24bc2c50b21e21b7U, 
  0x828db090e1cd730eU, 0xad3791da5c4463d6U, 0xb3e5bd6f53d634c8U, 0xc0408b712bcb6e23U, 0x4d3ad21d1fd87ea3U, 0x98446077a0fe0e8eU, 
  0xf748e97b646af62dU, 0x86f5e2308c2e69a4U, 0xd655a9b930b4ed3fU, 0x80a58947ca5967a6U,
};

/* how the values of gencrc[] were derived */
static void build_gencrc_table(void) {
  uint64_t tab[256];
  uint64_t i;

  /* fill tab[] with random permutations of 0..255 in each byte */
  for (i = 0; i < 256; i++) {
    unsigned int j;
    unsigned char x = (unsigned char) i;
    for (j = 0; j < 9; ++j) { x += 1; x += x << 1; x ^= x >> 1;}
    tab[i] = (uint64_t) x;
    for (j = 0; j < 9; ++j) { x += 2; x += x << 1; x ^= x >> 1;}
    tab[i] ^= (((uint64_t) x) << 8);
    for (j = 0; j < 9; ++j) { x += 3; x += x << 1; x ^= x >> 1;}
    tab[i] ^= (((uint64_t) x) << 16);
    for (j = 0; j < 9; ++j) { x += 4; x += x << 1; x ^= x >> 1;}
    tab[i] ^= (((uint64_t) x) << 24);
    for (j = 0; j < 9; ++j) { x += 5; x += x << 1; x ^= x >> 1;}
    tab[i] ^= (((uint64_t) x) << 32);
    for (j = 0; j < 9; ++j) { x += 6; x += x << 1; x ^= x >> 1;}
    tab[i] ^= (((uint64_t) x) << 40);
    for (j = 0; j < 9; ++j) { x += 7; x += x << 1; x ^= x >> 1;}
    tab[i] ^= (((uint64_t) x) << 48);
    for (j = 0; j < 9; ++j) { x += 8; x += x << 1; x ^= x >> 1;}
    tab[i] ^= (((uint64_t) x) << 56);
  }

  /* verify that 0..255 appears in each byte exactly once */
  for (i = 0; i < 256; i++) {
    unsigned int count, j;
    for (count = 0, j = 0; j < 256; ++j) if ((tab[j] & 0xffU) == i) ++count;
    if (count != 1) printf("bad1!  %"PRIu64" %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 8) & 0xffU) == i) ++count;
    if (count != 1) printf("bad2!  %"PRIu64" %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 16) & 0xffU) == i) ++count;
    if (count != 1) printf("bad3!  %"PRIu64" %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 24) & 0xffU) == i) ++count;
    if (count != 1) printf("bad4!  %"PRIu64" %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 32) & 0xffU) == i) ++count;
    if (count != 1) printf("bad5!  %"PRIu64" %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 40) & 0xffU) == i) ++count;
    if (count != 1) printf("bad6!  %"PRIu64" %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 48) & 0xffU) == i) ++count;
    if (count != 1) printf("bad7!  %"PRIu64" %d\n", i, count);
    for (count = 0, j = 0; j < 256; ++j) if (((tab[j] >> 56) & 0xffU) == i) ++count;
    if (count != 1) printf("bad8!  %"PRIu64" %d\n", i, count);
  }

  /* print out the table */
  for (i = 0; i < 256; ++i) {
    printf("0x%.16lxU, ", tab[i]);
    if (i%6 == 5) printf("\n");
  }
}

uint64_t gencrc(const char *key, unsigned int len,
		void *foo __attribute__ ((unused))) {
  uint64_t hash;
  unsigned int i;
  const unsigned char *k = (const unsigned char *)key;

  for (hash = (uint64_t) len, i = 0; i < len; i++)
    hash = (hash >> 8) ^ gencrctab[(hash & 0xffU) ^ k[i]];

  return hash;
}

#endif
