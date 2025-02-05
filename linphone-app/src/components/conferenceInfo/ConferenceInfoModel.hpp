﻿/*
 * Copyright (c) 2022 Belledonne Communications SARL.
 *
 * This file is part of linphone-desktop
 * (see https://www.linphone.org).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFERENCE_INFO_MODEL_H_
#define CONFERENCE_INFO_MODEL_H_

#include <linphone++/linphone.hh>
#include <QDateTime>
#include <QObject>
#include <QSharedPointer>
#include <QTimeZone>

class ParticipantListModel;
class ConferenceScheduler;
class TimeZoneModel;

class ConferenceInfoModel : public QObject {
	Q_OBJECT
	
public:
	Q_PROPERTY(TimeZoneModel * timeZoneModel READ getTimeZoneModel WRITE setTimeZoneModel NOTIFY timeZoneModelChanged)
	Q_PROPERTY(QDateTime dateTime READ getDateTimeSystem WRITE setDateTime NOTIFY dateTimeChanged)
	Q_PROPERTY(int duration READ getDuration WRITE setDuration NOTIFY durationChanged)
	Q_PROPERTY(QDateTime endDateTime READ getEndDateTime NOTIFY dateTimeChanged)
	Q_PROPERTY(QString organizer READ getOrganizer WRITE setOrganizer NOTIFY organizerChanged)
	Q_PROPERTY(QString subject READ getSubject WRITE setSubject NOTIFY subjectChanged)
	Q_PROPERTY(QString description READ getDescription WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(QString displayNamesToString READ displayNamesToString NOTIFY participantsChanged)
	Q_PROPERTY(QString uri READ getUri NOTIFY uriChanged)
	Q_PROPERTY(bool isScheduled READ isScheduled WRITE setIsScheduled NOTIFY isScheduledChanged)
		
	static QSharedPointer<ConferenceInfoModel> create(std::shared_ptr<linphone::ConferenceInfo> conferenceInfo);
	ConferenceInfoModel (QObject * parent = nullptr);
	ConferenceInfoModel (std::shared_ptr<linphone::ConferenceInfo> conferenceInfo, QObject * parent = nullptr);
	~ConferenceInfoModel ();
	std::shared_ptr<linphone::ConferenceInfo> getConferenceInfo();
	
//-------------------------------

	QDateTime getDateTimeUtc() const;
	QDateTime getDateTimeSystem() const;
	int getDuration() const;
	QDateTime getEndDateTime() const;
	QString getOrganizer() const;	
	QString getSubject() const;
	QString getDescription() const;
	Q_INVOKABLE QString displayNamesToString()const;
	QString getUri() const;
	bool isScheduled() const;
	Q_INVOKABLE QVariantList getParticipants() const;
	Q_INVOKABLE TimeZoneModel* getTimeZoneModel() const;
	
	void setDateTime(const QDateTime& dateTime);
	void setDuration(const int& duration);
	void setSubject(const QString& subject);
	void setOrganizer(const QString& organizerAddress);
	void setDescription(const QString& description);
	void setIsScheduled(const bool& on);
	
	Q_INVOKABLE void setParticipants(ParticipantListModel * participants);
	Q_INVOKABLE void setTimeZoneModel(TimeZoneModel * model);
	
// Tools
	Q_INVOKABLE void createConference(const int& securityLevel, const int& inviteMode);

// SCHEDULER
	
	virtual void onStateChanged(linphone::ConferenceSchedulerState state);
	virtual void onInvitationsSent(const std::list<std::shared_ptr<linphone::Address>> & failedInvitations);
	
	
signals:
	void timeZoneModelChanged();
	void dateTimeChanged();
	void durationChanged();
	void organizerChanged();
	void subjectChanged();
	void descriptionChanged();
	void participantsChanged();
	void uriChanged();
	void isScheduledChanged();
	
	void conferenceCreated();
	void conferenceCreationFailed();
	void invitationsSent();
	
private:
	std::shared_ptr<linphone::ConferenceInfo> mConferenceInfo;
	QSharedPointer<ConferenceScheduler> mConferenceScheduler= nullptr;
	QTimeZone mTimeZone;
	
	bool mIsScheduled = true;
};

Q_DECLARE_METATYPE(QSharedPointer<ConferenceInfoModel>)
Q_DECLARE_METATYPE(ConferenceInfoModel*)

#endif
