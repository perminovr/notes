#include "mymodbustcpserver.h"
#include <QDebug>

MyModbusTcpServer::MyModbusTcpServer(QObject *parent) : QModbusTcpServer(parent)
{}


bool MyModbusTcpServer::writeData(const QModbusDataUnit &newData)
{
    qInfo() << "writeData" << endl;


    if (!m_modbusDataUnitMultiMap.contains(newData.registerType()))
        return false;

    bool hit = false;
    bool changeRequired = false;
    auto allReg = m_modbusDataUnitMultiMap.equal_range(newData.registerType());

    for (auto it = allReg.first; it != allReg.second; ++it) {
        QModbusDataUnit &current = *it;
        if (!current.isValid()) {
            continue;
        }

        // check range start is within internal map range
        int internalRangeEndAddress = current.startAddress() + current.valueCount() - 1;
        if (newData.startAddress() < current.startAddress() || newData.startAddress() > internalRangeEndAddress) {
            continue;
        }

        // check range end is within internal map range
        int rangeEndAddress = newData.startAddress() + newData.valueCount() - 1;
        if (rangeEndAddress < current.startAddress() || rangeEndAddress > internalRangeEndAddress) {
            continue;
        }

        hit = true;

        for (uint i = 0; i < newData.valueCount(); i++) {
            const quint16 newValue = newData.value(i);
            const int translatedIndex = newData.startAddress() - current.startAddress() + i;
            changeRequired |= (current.value(translatedIndex) != newValue);
            current.setValue(translatedIndex, newValue);
        }
    }

    if (changeRequired)
        emit dataWritten(newData.registerType(), newData.startAddress(), newData.valueCount());

    return hit;
}


bool MyModbusTcpServer::readData(QModbusDataUnit *newData) const
{
    qInfo() << "readData" << endl;

    if ((!newData) || (!m_modbusDataUnitMultiMap.contains(newData->registerType())))
        return false;

    bool hit = false;
    auto allReg = m_modbusDataUnitMultiMap.equal_range(newData->registerType());

    for (auto it = allReg.first; it != allReg.second; ++it) {
        const QModbusDataUnit &current = *it;
        if (!current.isValid()) {
            continue;
        }

        // check range start is within internal map range
        int internalRangeEndAddress = current.startAddress() + current.valueCount() - 1;
        if (newData->startAddress() < current.startAddress() || newData->startAddress() > internalRangeEndAddress) {
            continue;
        }

        // check range end is within internal map range
        const int rangeEndAddress = newData->startAddress() + newData->valueCount() - 1;
        if (rangeEndAddress < current.startAddress() || rangeEndAddress > internalRangeEndAddress) {
            continue;
        }

        hit = true;

        newData->setValues(current.values().mid(newData->startAddress() - current.startAddress(), newData->valueCount()));
    }
    return hit;
}


bool MyModbusTcpServer::setMap(const QModbusDataUnitMultiMap &map)
{
    m_modbusDataUnitMultiMap = map;
    return true;
}
