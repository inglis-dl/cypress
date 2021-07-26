#pragma once

#include "InputsModel.h"
#include "OutputsModel.h"
#include "ui_MainWindow.h"

#include <QtWidgets/QMainWindow>

class FraxUI
{
public:
	static void SetInputs(Ui::FraxClass* ui, InputsModel* inputs);
	static void SetOutputs(Ui::FraxClass* ui, OutputsModel* outputs);
};

