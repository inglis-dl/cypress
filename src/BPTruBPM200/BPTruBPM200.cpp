#include "BPTruBPM200.h"

BPTruBPM200::BPTruBPM200(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	bpmCommunication.StartBPMCommunication();
	SetupSlots();
	//testHidApi();
}

void BPTruBPM200::closeEvent(QCloseEvent* event) {
	bpmCommunication.StopBPMCommunication();
	event->accept();
}

int BPTruBPM200::testHidApi()
{
	//hid_device* handle;

	//struct hid_device_info* devs, * cur_dev;
	qDebug() << "hello from test hid" << endl;

	if (hid_init())
		return -1;
	qDebug() << "hid init passed" << endl;

	/*devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		qDebug() << "Device Found\n " << endl;
		qDebug() << "type:          " << cur_dev->vendor_id << cur_dev->product_id << endl;
		qDebug() << "path:          " << cur_dev->path << endl;
		qDebug() << "serial_number: " << cur_dev->serial_number << endl;
		qDebug() << "\n";
		qDebug() << " Manufacturer:" << cur_dev->manufacturer_string << endl;
		qDebug() << " Product:     " << cur_dev->product_string << endl;
		qDebug() << " Release:     " << cur_dev->release_number << endl;
		qDebug() << " Interface:   " << cur_dev->interface_number << endl;
		qDebug() << " Usage (page):" << cur_dev->usage << " (" << cur_dev->usage_page << ")" << endl;
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);*/

	/*QString filename = "C:/Users/clsa/Desktop/bpmTestReads.txt";
	QFile file(filename);

	hid_device* bpm = hid_open(4279, 4660, 0);
	//hid_set_nonblocking(bpm, 1);
	const unsigned char msg[] = { 0x11, 0x04, 0x00, 0x00, 0x00};
	unsigned char crc = CRC8::Calculate(msg,5);
	qDebug() << crc << endl;
	const unsigned char data[] = {0x00, 0x02, msg[0], msg[1], msg[2], msg[3], msg[4], crc, 0x03};
	hid_write(bpm, data, 9);

	while (true) {
		const int dataLength = 1024;
		unsigned char readData[dataLength];
		int numBytesReturned = hid_read(bpm, readData, dataLength);
		qDebug() << "NumBytes: " << numBytesReturned << endl;
		if (numBytesReturned > 0) {
			bool opened = file.open(QIODevice::Append);
			qDebug() << opened << endl;
			QTextStream stream(&file);
			for (int i = 0; i < numBytesReturned;) {
				for (int j = 0; j < 8; j++) {
					qDebug() << j << " : " << readData[i] << endl;
					stream << readData[i] << ",";
					i++;
				}
				stream << "\r\n";
			}
			stream.flush();
			file.close();
		}
	}*/


	///* Free static HIDAPI objects. */
	hid_exit();
	qDebug() << "hid exit passed" << endl;
	return 0;
}

void BPTruBPM200::SetupSlots()
{
    connect(ui.StartButton, SIGNAL(clicked()), this, SLOT(OnStartClicked()));
    connect(ui.StopButton, SIGNAL(clicked()), this, SLOT(OnStopClicked()));
    connect(ui.ClearButton, SIGNAL(clicked()), this, SLOT(OnClearClicked()));
    connect(ui.ReviewButton, SIGNAL(clicked()), this, SLOT(OnReviewClicked()));
    connect(ui.CycleButton, SIGNAL(clicked()), this, SLOT(OnCycleClicked()));
}

void BPTruBPM200::OnStartClicked() {
    ui.ErrorsTextBrowser->setText("Start Pressed");
	bpmCommunication.AddToWriteQueue(BPMCommands::NIBPStart());
}

void BPTruBPM200::OnStopClicked() {
    ui.ErrorsTextBrowser->setText("Stop Pressed");
	bpmCommunication.AddToWriteQueue(BPMCommands::NIBPStop());
}

void BPTruBPM200::OnClearClicked() {
    ui.ErrorsTextBrowser->setText("Clear Pressed");
	bpmCommunication.AddToWriteQueue(BPMCommands::NIBPClear());
}

void BPTruBPM200::OnReviewClicked() {
    ui.ErrorsTextBrowser->setText("Review Pressed");
	bpmCommunication.AddToWriteQueue(BPMCommands::NIBPReview());
}

void BPTruBPM200::OnCycleClicked() {
    ui.ErrorsTextBrowser->setText("Cycle Pressed");
	bpmCommunication.AddToWriteQueue(BPMCommands::NIBPCycle());
}
