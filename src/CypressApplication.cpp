#include "CypressApplication.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <stdexcept>

#include "./auxiliary/Constants.h"
#include "./dialogs/DialogFactory.h"
#include "./dialogs/DialogBase.h"

CypressApplication::CypressApplication(QObject *parent) : QObject(parent)
{
}

CypressApplication::~CypressApplication()
{
}

void CypressApplication::setArgs(const QMap<QString, QVariant> &args)
{
    if(args.contains("inputFileName"))
      m_inputFileName = args["inputFileName"].toString();
    if(args.contains("outputFileName"))
      m_outputFileName = args["outputFileName"].toString();
    if(args.contains("measureType"))
      m_type = args["measureType"].value<Constants::MeasureType>();
    if(args.contains("runMode"))
      m_mode = args["runMode"].value<Constants::RunMode>();
    if(args.contains("verbose"))
      m_verbose = args["verbose"].toBool();
}

void CypressApplication::initialize()
{
    DialogFactory *factory = DialogFactory::instance();
    m_dialog.reset(factory->instantiate(m_type));

    if(m_dialog.isNull())
        throw std::runtime_error("FATAL ERROR: failed to initialize a dialog");

    m_dialog->setInputFileName(m_inputFileName);
    m_dialog->setOutputFileName(m_outputFileName);
    m_dialog->setRunMode(m_mode);
    m_dialog->setVerbose(m_verbose);
    m_dialog->initialize();
    m_dialog->show();
}
