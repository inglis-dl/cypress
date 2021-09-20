//#include "IBPMCommands.h"

class BPMCommands{
public:
	static BPMMessage Reset() { return BPMMessage(0x10, 0xff); };
	static BPMMessage HandShake() { return BPMMessage(0x11, 0x00); };
	static BPMMessage NIBPStop() { return BPMMessage(0x11, 0x01); };
	static BPMMessage NIBPReview() { return BPMMessage(0x11, 0x02); };
	static BPMMessage NIBPCycle() { return BPMMessage(0x11, 0x03); };
	static BPMMessage NIBPStart() { return BPMMessage(0x11, 0x04); };
	static BPMMessage NIBPClear() { return BPMMessage(0x11, 0x05); };
	static BPMMessage DisablePressures() { return BPMMessage(0x11, 0x08, 0x01); };
	static BPMMessage EnablePressures() { return BPMMessage(0x11, 0x08); };
	static BPMMessage RetrieveLastResult() { return BPMMessage(0x11, 0x09); };
};