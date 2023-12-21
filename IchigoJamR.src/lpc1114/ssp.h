#ifndef SSP_DEFINED
#define SSP_DEFINED

// SSP Status register
#define SSPSR_TFE	(1 << 0)
#define SSPSR_TNF	(1 << 1) 
#define SSPSR_RNE	(1 << 2)
#define SSPSR_RFF	(1 << 3) 
#define SSPSR_BSY	(1 << 4)
// SSP CR1 register
#define SSPCR1_LBM	(1 << 0)
#define SSPCR1_SSE	(1 << 1)
#define SSPCR1_MS	(1 << 2)
#define SSPCR1_SOD	(1 << 3)

#endif
