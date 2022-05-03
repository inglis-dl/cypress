#pragma once

#include <QString>
#include <QJsonObject>
#include <QFile>
#include <QList>

typedef QList<QJsonObject> q_paradoxRecords;
typedef QList<q_paradoxRecords> q_paradoxBlocks;

enum class FieldType
{
    Alpha = 0x01,
    Date = 0x02,
    ShortInteger = 0x03,
    LongInteger = 0x04,
    Currency = 0x05,
    Number = 0x06,
    Logical = 0x09,
    MemoBlob = 0x0C,
    BLOB = 0x0D,
    FormatedMemoBlod = 0x0E,
    OLE = 0x0F,
    GraphicBlob = 0x10,
    Time = 0x14,
    Timestamp = 0x15,
    AutoInc = 0x16,
    BCD = 0x17,
    Bytes = 0x18
};

class DbHeaderField
{
public:
    DbHeaderField(FieldType type, int size)
    {
        m_type = type;
        m_size = size;
    }
    FieldType getType() const { return m_type; }
    int getSize() const { return m_size; }
    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
private:
    FieldType m_type;
    int m_size;
    QString m_name;
};

class DbHeader
{
public:
    // Directly from bytes
    qint16 recordSize;
    qint16 headerSize;
    quint8 fileType;
    quint8 maxTableSize;
    qint32 numRecords;
    quint16 nextBlock;
    quint16 fileBlocks;
    quint16 firstBlock;
    quint16 lastBlock;
    quint8 modifiedFlags1;
    quint8 indexFieldNumber;
    qint32 primaryIndexWorkspace;
    qint16 numFields;
    qint16 primaryKeyFields;
    qint32 encryption1;
    quint8 sortOrder;
    quint8 modifiedFlags2;
    quint8 changeCount1;
    quint8 changeCount2;
    qint32 tableNamePtr;
    qint32 fieldInfoPtr;
    quint8 writeProtected;
    quint8 fileVersionId;
    quint16 maxBlocks;
    quint8 auxPassword;
    qint32 cryptInfoStartPtr;
    qint32 cryptInfoEndPtr;
    qint32 autoInc;
    quint8 indexUpdateRequired;
    quint8 refIntegrity;

    // Not directly from bytes
    QList<DbHeaderField> fields;
};

class ParadoxDbBlock {
public:
    ParadoxDbBlock(const int& blockNumber, const int& nextBlock, const int& prevBlock,
        const int& offsetToLastRecord, const long& fileOffset, const DbHeader* header) :
        m_blockNumber(blockNumber),
        m_nextBlock(nextBlock),
        m_prevBlock(prevBlock),
        m_offsetToLastRecord(offsetToLastRecord),
        m_fileOffset(fileOffset),
        m_dbHeader(header)
    {
    }

    // offsetToLastRecord is set to -header.recordSize when the block is empty.
    // this method will thus return 0 in this case
    int getNumRecords() { return (m_offsetToLastRecord / m_dbHeader->recordSize) + 1; }

    QList<QJsonObject> readRecords(QFile& dbFile)
    {
        int numRecords = getNumRecords();
        QList<QJsonObject> records;
        dbFile.seek(m_fileOffset);
        for (int i = 0; i < numRecords; i++) {
            QJsonObject record = readRecord(dbFile);
            records.append(record);
        }
        return records;
    }

private:
    const int m_blockNumber;
    const int m_nextBlock;
    const int m_prevBlock;
    const int m_offsetToLastRecord;
    const long m_fileOffset;
    const DbHeader* m_dbHeader;

    QJsonObject readRecord(QFile& dbFile) {
        QJsonObject jsonRecord;
        for (int i = 0; i < m_dbHeader->numFields; i++) {
            DbHeaderField field = m_dbHeader->fields[i];
            FieldType type = field.getType();
            QByteArray bytes = dbFile.read(field.getSize());
            QVariant value = getValue(bytes, type);
            jsonRecord.insert(field.getName(), QJsonValue::fromVariant(value));
        }
        return jsonRecord;
    }

    QVariant getValue(QByteArray bytes, FieldType type) {
        QVariant output;

        switch (type) {
        case FieldType::Alpha:
        {
            QString result;
            result.reserve(bytes.size());
            for (int i = 0; i < bytes.size(); i++) {
                result.append((char)bytes[i]);
            }
            output = result;
            break;
        }
        case FieldType::LongInteger:
        {
            QByteArray fixed = fixSign(bytes);
            long value = 0;
            int leftShift = (bytes.size() - 1) * 8;
            int mask = 0x000000FF;
            for (int i = 0; i < bytes.size(); i++) {
                value |= fixed[i] << leftShift & (mask << leftShift);
                leftShift = -8;
            }
            output = (bytes[0] & 0x80) == 0x80 ? value : (value == 0 ? NULL : -value);
            break;
        }
        // These two are in the data set, but ONYX has them return null
        // So likely not needed to use the data from these types
        case FieldType::Logical:
        case FieldType::MemoBlob:
            break;
        default:
            break;
        }
        return output;
    }

    // Using QByteArray instead of QByteArray& because a copy 
    // is required and do not want to alias
    QByteArray fixSign(QByteArray bytes) {
        if (bytes[0] & 0x80) {
            //bytes[0] &= 0x7f; This is the java code. not too sure about &=
            bytes[0] = bytes[0] & 0x7f;
        }
        return bytes;
    }
};

class ParadoxReader
{
public:
    ParadoxReader(const QString& filePath, QWidget* parent = Q_NULLPTR);
    ~ParadoxReader();

    q_paradoxBlocks Read();
    void closeDatabase();
private:
    void openDatabase(const QString& filePath);

    // Methods for reading in header data
    void readHeader();
    bool headerDataValid();
    void seekToFieldInfoStart();
    void readHeaderFieldInfo();
    void skipPastUneededInfo();
    void readHeaderFieldNames();

    // Read blocks of data
    QList<ParadoxDbBlock> readBlocks();

    // Helper methods for reading various types of data
    qint16 readShort(int offset = -1);
    quint8 readUByte(int offset = -1);
    qint32 readInt(int offset = -1);
    quint16 readUShort(int offset = -1);
    qint32 readPtr(int offset = -1);

    void handleError(QJsonObject* json, const QString& errorMsg);
   // void writeBlockInJson(QJsonObject* json, QList<ParadoxRecord> records, int blockNum);

    QFile m_dbFile;
    DbHeader m_header;
};

