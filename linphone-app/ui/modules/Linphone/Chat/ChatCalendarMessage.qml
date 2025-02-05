import QtQuick 2.7
import QtQuick.Layouts 1.3

import Clipboard 1.0
import App 1.0
import Common 1.0
import Linphone 1.0

import Common.Styles 1.0
import Linphone.Styles 1.0
import TextToSpeech 1.0
import Utils 1.0
import Units 1.0
import UtilsCpp 1.0
import LinphoneEnums 1.0

import ColorsList 1.0

import 'Message.js' as Logic
import 'qrc:/ui/scripts/Utils/utils.js' as Utils

// =============================================================================

Loader{
	id: mainItem
	property ContentModel contentModel
	property ConferenceInfoModel conferenceInfoModel: contentModel ? contentModel.conferenceInfoModel : null
	property int maxWidth : parent.width
	property int fitHeight: active && item ? item.fitHeight + ChatCalendarMessageStyle.topMargin+ChatCalendarMessageStyle.bottomMargin + (isExpanded? 200 : 0): 0
	property int fitWidth: active && item ? maxWidth/2  + ChatCalendarMessageStyle.widthMargin*2 : 0
	property bool containsMouse: false
	property int gotoButtonMode: -1	//-1: hide, 0:goto, 1:MoreInfo
	property bool isExpanded : false
	
	signal expandToggle()
	signal conferenceUriCopied()
	
	width: parent.width
	height: parent.height
	
	property font customFont : SettingsModel.textMessageFont
	active: mainItem.conferenceInfoModel
	
	sourceComponent: MouseArea{
		id: loadedItem
		property int fitHeight: layout.fitHeight + ChatCalendarMessageStyle.topMargin+ChatCalendarMessageStyle.bottomMargin
		property int fitWidth: layout.fitWidth
		
		anchors.fill: parent
		anchors.leftMargin: ChatCalendarMessageStyle.widthMargin
		anchors.rightMargin: ChatCalendarMessageStyle.widthMargin
		anchors.topMargin: ChatCalendarMessageStyle.topMargin
		anchors.bottomMargin: ChatCalendarMessageStyle.bottomMargin
		
		clip: false
		
		hoverEnabled: true
		onClicked: CallsListModel.prepareConferenceCall(mainItem.conferenceInfoModel)
		onHoveredChanged: mainItem.containsMouse = loadedItem.containsMouse		
		
		ColumnLayout{
			id: layout
			property int fitHeight: Layout.minimumHeight
			property int fitWidth: Layout.minimumWidth
			anchors.fill: parent
			spacing: 0
			RowLayout {
				Layout.fillWidth: true
				Layout.preferredWidth: parent.width	// Need this because fillWidth is not enough...
				Layout.preferredHeight: ChatCalendarMessageStyle.lineHeight
				Layout.topMargin: 5
				spacing: 10
				RowLayout {
					id: scheduleRow
					Layout.fillWidth: true
					Layout.preferredHeight: ChatCalendarMessageStyle.lineHeight
					Layout.leftMargin: 5
					spacing: ChatCalendarMessageStyle.schedule.spacing
					
					Icon{
						icon: ChatCalendarMessageStyle.schedule.icon
						iconSize: ChatCalendarMessageStyle.schedule.iconSize
						overwriteColor: ChatCalendarMessageStyle.schedule.color
					}
					
					Text {
						id: conferenceTime
						Layout.fillWidth: true
						Layout.minimumWidth: implicitWidth
						verticalAlignment: Qt.AlignVCenter
						color: ChatCalendarMessageStyle.schedule.color
						elide: Text.ElideRight
						font.pointSize: ChatCalendarMessageStyle.schedule.pointSize
						text: Qt.formatDateTime(mainItem.conferenceInfoModel.dateTime, 'hh:mm')
							  +' - ' +Qt.formatDateTime(mainItem.conferenceInfoModel.endDateTime, 'hh:mm')
					}
				}
				Text{
					Layout.fillWidth: true
					Layout.rightMargin: 15
					horizontalAlignment: Qt.AlignRight
					verticalAlignment: Qt.AlignVCenter
					color: ChatCalendarMessageStyle.schedule.color
					elide: Text.ElideRight
					font.pointSize: ChatCalendarMessageStyle.schedule.pointSize
					//: 'Organizer' : Label Title for the organizer.
					text: qsTr('icsOrganizer') +' : ' +UtilsCpp.getDisplayName(mainItem.conferenceInfoModel.organizer)
				}
			}
			Text{
				id: title
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.alignment: Qt.AlignRight
				elide: Text.ElideRight
				color: ChatCalendarMessageStyle.subject.color
				font.pointSize: ChatCalendarMessageStyle.subject.pointSize
				font.weight: Font.Bold
				text: mainItem.conferenceInfoModel.subject
			}
			RowLayout {
				id: participantsRow
				Layout.fillWidth: true
				Layout.preferredHeight: ChatCalendarMessageStyle.participants.iconSize
				Layout.leftMargin: 5
				Layout.rightMargin: 15
				
				spacing: ChatCalendarMessageStyle.participants.spacing
				
				Icon{
					icon: ChatCalendarMessageStyle.participants.icon
					iconSize: ChatCalendarMessageStyle.participants.iconSize
					overwriteColor: ChatCalendarMessageStyle.participants.color
				}
				
				Text {
					id: participantsList
					Layout.fillWidth: true
					color: ChatCalendarMessageStyle.participants.color
					elide: Text.ElideRight
					font.pointSize: ChatCalendarMessageStyle.participants.pointSize
					text: mainItem.conferenceInfoModel.displayNamesToString
				}	
				ActionButton{
					visible: mainItem.gotoButtonMode >= 0
					Layout.preferredHeight: iconSize
					Layout.preferredWidth: height
					isCustom: true
					colorSet: mainItem.gotoButtonMode == 0 ? ChatCalendarMessageStyle.gotoButton : ChatCalendarMessageStyle.infoButton
					backgroundRadius: width/2
					toggled: mainItem.isExpanded
					onClicked: mainItem.expandToggle()
				}
			}
			ColumnLayout{
				id: expandedDescription
				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.topMargin: 5
				visible: mainItem.isExpanded
				spacing: 0
				ScrollableListView{
					id: expandedParticipantsList
					Layout.fillWidth: true
					Layout.minimumHeight: Math.min( count * ChatCalendarMessageStyle.lineHeight, parent.height/(descriptionTitle.visible?3:2))
					Layout.leftMargin: 10
					spacing: 0
					visible: mainItem.isExpanded
					onVisibleChanged: model= mainItem.conferenceInfoModel.getParticipants()
					
					delegate: Row{
						spacing: 5
						width: expandedParticipantsList.width
						height: ChatCalendarMessageStyle.lineHeight
						Text{text: modelData.displayName
							color: ChatCalendarMessageStyle.description.color
							font.pointSize: ChatCalendarMessageStyle.description.pointSize
							elide: Text.ElideRight
							wrapMode: TextEdit.WordWrap
						}
						Text{text: '('+modelData.address+')'
							color: ChatCalendarMessageStyle.description.color
							font.pointSize: ChatCalendarMessageStyle.description.pointSize
							elide: Text.ElideRight
							wrapMode: TextEdit.WordWrap
						}
					}
				}
				Text{
					id: descriptionTitle
					Layout.fillWidth: true
					Layout.leftMargin: 10
					Layout.topMargin: 5
					color: ChatCalendarMessageStyle.subject.color
					font.pointSize: ChatCalendarMessageStyle.subject.pointSize
					font.weight: Font.Bold
					//: 'Description' : Title for the conference description.
					text: qsTr('icsDescription')
					visible: description.text != ''
				}
				TextAreaField{
					id: description
					Layout.fillWidth: true
					Layout.fillHeight: true
					Layout.leftMargin: 10
					padding: 0
					color: 'transparent'
					readOnly: true
					textColor: ChatCalendarMessageStyle.description.color
					font.pointSize: ChatCalendarMessageStyle.description.pointSize
					border.width: 0
					visible: description.text != ''
					
					text: mainItem.conferenceInfoModel.description
				}
				Item{
					Layout.fillHeight: true
					Layout.fillWidth: true
				}
				Text{
					id: linkTitle
					Layout.fillWidth: true
					Layout.leftMargin: 10
					color: ChatCalendarMessageStyle.subject.color
					font.pointSize: ChatCalendarMessageStyle.subject.pointSize
					font.weight: Font.Bold
					
					//: 'Conference address' : Title for the conference address.
					text: qsTr('icsconferenceAddressTitle')
				}
				RowLayout{
					Layout.fillWidth: true
					Layout.fillHeight: true
					Layout.leftMargin: 10
					Layout.rightMargin: 60
					spacing: 10
					TextField{
						id: uriField
						readOnly: true
						Layout.fillWidth: true
						textFieldStyle: TextFieldStyle.flatInverse
						text: mainItem.conferenceInfoModel.uri
						
					}
					ActionButton{
						iconSize: uriField.height
						isCustom: true
						colorSet: ChatCalendarMessageStyle.copyLinkButton
						backgroundRadius: width/2
						onClicked: {
								Clipboard.text = uriField.text
								mainItem.conferenceUriCopied()
							}
					}
				}
				RowLayout{
					Layout.fillWidth: true
					Layout.topMargin: 10
					Layout.rightMargin: 10
					spacing: 10
					Item{
						Layout.fillWidth: true
					}
					TextButtonC{
						//: 'Join' : Action button to join the conference.
						text: qsTr('icsJoinButton').toUpperCase()
						onClicked: CallsListModel.prepareConferenceCall(mainItem.conferenceInfoModel)
					}
					ActionButton{
						isCustom: true
						colorSet: ChatCalendarMessageStyle.editButton
						backgroundRadius: width/2
						onClicked: {
							window.detachVirtualWindow()
							window.attachVirtualWindow(Utils.buildAppDialogUri('NewConference')
													   ,{conferenceInfoModel: mainItem.conferenceInfoModel})
						}
					}
					ActionButton{
						isCustom: true
						colorSet: ChatCalendarMessageStyle.deleteButton
						backgroundRadius: width/2
					}
				}
			}
		}
	}
}

