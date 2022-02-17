#include "CypressApplication.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "./auxiliary/CypressConstants.h"
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
    if(args.contains("type"))
      m_type = args["type"].value<CypressConstants::Type>();
    if(args.contains("mode"))
      m_mode = args["mode"].value<CypressConstants::Mode>();
    if(args.contains("verbose"))
      m_verbose = args["verbose"].toBool();
}

void CypressApplication::initialize()
{
    DialogFactory *df = DialogFactory::instance();
    m_dialog.reset(df->instantiate(m_type));

    if(m_dialog.isNull())
        throw std::runtime_error("FATAL ERROR: failed to initialize a dialog");

    m_dialog->setInputFileName(m_inputFileName);
    m_dialog->setOutputFileName(m_outputFileName);
    m_dialog->setMode(m_mode);
    m_dialog->setVerbose(m_verbose);
    m_dialog->initialize();
    m_dialog->show();
}
