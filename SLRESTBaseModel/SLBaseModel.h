/*
 * Base.h
 *
 *  Created on: 27.03.2013
 *      Author: oliver
 */

#ifndef SL_BASE_MODEL_H_
#define SL_BASE_MODEL_H_

#include <QtCore/QObject>
#include <QMap>
#include "dquest.h"
#include "dqmetainfoquery_p.h"

template <class T>
class SLBaseModel : public DQModel {

public:
	SLBaseModel() : DQModel() {};
	virtual ~SLBaseModel() {};

	virtual QMap<QString, QString> attributeMapping(void) =0;
	void updateWithJSONObject(const QVariant& JSONObject);
	static T UpdatedObjectWithJSONObject(const QVariant& JSONObject);
	static QList<T> ParseJSON(const QVariant& JSONObject);
	QVariantMap toVariant(void);

	static QList<T> Find(void);
	static QList<T> Find(DQWhere where);
};

template <class T>
void SLBaseModel<T>::updateWithJSONObject(const QVariant& JSONObject) {
	QMap<QString, QString> mapping = this->attributeMapping();
	QList<QString> JSONKeys = mapping.keys();
	QMap<QString, QVariant> attributes = JSONObject.toMap();

	QList<QString> dateKeys;

	for (int i = 0; i < this->metaInfo()->size(); ++i) {
		if (this->metaInfo()->at(i)->type == QVariant::DateTime) {
			dateKeys.append(this->metaInfo()->at(i)->name);
		}
	}

	QList<QString>::iterator i = JSONKeys.begin();
	while (i != JSONKeys.end()) {
		QString key = *i;
		QVariant value = attributes[key];

		QByteArray array = mapping[key].toLocal8Bit();
		const char *propertyName = array.data();

//		qDebug() << "assining " << value << " for " << propertyName;
		if (dateKeys.contains(propertyName)) {
			QDateTime dateTime = QDateTime::fromString(value.toString(), Qt::ISODate);
			bool success = this->metaInfo()->setValue(this, propertyName, dateTime);
			Q_ASSERT(success);
		} else {
			bool success = this->metaInfo()->setValue(this, propertyName, value);
			Q_ASSERT(success);
		}

		++i;
	}
}

template <class T>
T SLBaseModel<T>::UpdatedObjectWithJSONObject(const QVariant& JSONObject)
{
	T object;
	QMap<QString, QVariant> attributes = JSONObject.toMap();
	QVariant identifier = attributes["id"];

	object.load(DQWhere("identifier") == identifier);

	object.updateWithJSONObject(JSONObject);
	object.save();

	return object;
}

template <class T>
QList<T> SLBaseModel<T>::ParseJSON(const QVariant& JSONObject)
{
	QList<T> result;

	QList<QVariant> listOfJSONObjects = JSONObject.toList();
	if (listOfJSONObjects.count() > 0) {
		// is a list of JSON objects
		QList<QVariant>::Iterator i = listOfJSONObjects.begin();
		while (i != listOfJSONObjects.end()) {
			result << UpdatedObjectWithJSONObject(*i);
			++i;
		}
	} else {
		// try to parse single JSON object
		result << UpdatedObjectWithJSONObject(JSONObject);
	}

	return result;
}

template <class T>
QVariantMap SLBaseModel<T>::toVariant(void)
{
	QVariantMap map;

	for (int i = 0; i < this->metaInfo()->size(); ++i) {
		const DQModelMetaInfoField *field = this->metaInfo()->at(i);
		QVariant value = this->metaInfo()->value(this, field->name);
		map[field->name] = value;
	}

	return map;
}

template <class T>
QList<T> SLBaseModel<T>::Find(void)
{
	QList<T> result;
	T *dummy = new T();
	_DQMetaInfoQuery query(dummy->metaInfo(), dummy->connection());

	if (query.exec()) {
		while (query.next()) {
			T object;
			if (query.recordTo(&object)) {
				result.append(object);
			}
		}
	}

	dummy->connection().setLastQuery(query.lastQuery());
	delete dummy;
    return result;
}

template <class T>
QList<T> SLBaseModel<T>::Find(DQWhere where)
{
	QList<T> result;
	T *dummy = new T();
	_DQMetaInfoQuery query(dummy->metaInfo(), dummy->connection());

	query = query.filter(where);
	if (query.exec()) {
		while (query.next()) {
			T object;
			if (query.recordTo(&object)) {
				result.append(object);
			}
		}
	}

	dummy->connection().setLastQuery(query.lastQuery());
	delete dummy;
    return result;
}

#endif /* SL_BASE_MODEL_H_ */
