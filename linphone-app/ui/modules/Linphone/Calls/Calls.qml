import QtQuick 2.7

import Common 1.0
import Linphone 1.0
import Linphone.Styles 1.0
import ColorsList 1.0

import 'Calls.js' as Logic

// =============================================================================

ListView {
	id: calls
	
	// ---------------------------------------------------------------------------
	
	readonly property CallModel selectedCall: calls._selectedCall
	
	property var conferenceModel
	
	property CallModel _selectedCall: null
	
	// ---------------------------------------------------------------------------
	
	boundsBehavior: Flickable.StopAtBounds
	clip: true
	spacing: 0
	
	// ---------------------------------------------------------------------------
	
	onCountChanged: Logic.handleCountChanged(count)
	
	Connections {
		target: model
		
		onCallRunning: Logic.handleCallRunning(callModel)
		onRowsAboutToBeRemoved: Logic.handleRowsAboutToBeRemoved(parent, first, last)
		onRowsInserted: Logic.handleRowsInserted(parent, first, last)
	}
	
	// ---------------------------------------------------------------------------
	
	Component {
		id: callAction
		
		ActionButton {
			isCustom: true
			backgroundRadius: 90
			Component.onCompleted: params.useIcon == 1 ? colorSet = CallsStyle.entry.hangup : errorIcon.visible=true
			
			onClicked: params.handler()
			Icon{
				id: errorIcon
				icon: 'generic_error'
				iconSize: CallsStyle.entry.iconActionSize
				visible: false
			}
		}
	}
	
	Component {
		id: callActions
		
		ActionButton {
			id: button
			property bool isSelected : calls.currentIndex === callId && call.status !== CallModel.CallStatusEnded
			isCustom: true
			backgroundRadius: 4
			colorSet: isSelected ? CallsStyle.entry.selectedBurgerMenu : CallsStyle.entry.burgerMenu
			
			onClicked: menu.open()
			
			DropDownStaticMenu {
				id: menu
				
				relativeTo: callControls
				relativeX: callControls.width
				
				entryHeight: CallsStyle.entry.height
				entryWidth: maxWidth 
				property int maxWidth :  CallsStyle.entry.width
				
				Repeater {
					model: params ? params.actions : []
					DropDownStaticMenuEntry {
						entryName: modelData.name
						Component.onCompleted: if( menu.maxWidth < implicitWidth ) menu.maxWidth = implicitWidth
						onClicked: {
							menu.close()
							params.actions[index].handler()
						}
					}
				}
			}
		}
	}
	
	// ---------------------------------------------------------------------------
	// Conference.
	// ---------------------------------------------------------------------------
	
	header: ConferenceControls {
		readonly property bool isSelected: calls.currentIndex === -1 && calls._selectedCall == null
		
		height: visible ? ConferenceControlsStyle.height : 0
		width: parent.width
		
		visible: calls.conferenceModel.count > 0
		
		color: isSelected
			   ? CallsStyle.entry.color.selected
			   : CallsStyle.entry.color.normal
		
		textColor: isSelected
				   ? CallsStyle.entry.usernameColor.selected
				   : CallsStyle.entry.usernameColor.normal
		
		onClicked: Logic.resetSelectedCall()
		onVisibleChanged: !visible && Logic.handleCountChanged(calls.count)
	}
	
	// ---------------------------------------------------------------------------
	// Calls.
	// ---------------------------------------------------------------------------
	
	delegate: CallControls {
		id: _callControls
		
		// -------------------------------------------------------------------------
		
		function useColorStatus () {
			return calls.currentIndex === index && $modelData && $modelData.status !== CallModel.CallStatusEnded
		}
		
		// -------------------------------------------------------------------------
		
		color: useColorStatus()
			   ? CallsStyle.entry.color.selected
			   : CallsStyle.entry.color.normal
		sipAddressColor: useColorStatus()
						 ? CallsStyle.entry.sipAddressColor.selected
						 : CallsStyle.entry.sipAddressColor.normal
		usernameColor: useColorStatus()
					   ? CallsStyle.entry.usernameColor.selected
					   : CallsStyle.entry.usernameColor.normal
		
		signIcon: {
			var params = loader.params
			return params ? 'call_sign_' + params.string : ''
		}
		
		entry: $modelData
		
		width: calls.width
		
		onClicked: {
			if ($modelData.status !== CallModel.CallStatusEnded) {
				Logic.updateSelectedCall($modelData, index)
			}
		}
		
		// -------------------------------------------------------------------------
		
		Loader {
			id: loader
			
			readonly property int callId: index
			
			readonly property var call: $modelData
			readonly property var callControls: _callControls
			readonly property var params: Logic.getParams($modelData)
			
			anchors.centerIn: parent
			sourceComponent: params ? params.component : null
		}
		
		SequentialAnimation on color {
			loops: CallsStyle.entry.endCallAnimation.loops
			running: !$modelData || $modelData.status === CallModel.CallStatusEnded
			
			ColorAnimation {
				duration: CallsStyle.entry.endCallAnimation.duration
				from: CallsStyle.entry.color.normal
				to: CallsStyle.entry.endCallAnimation.blinkColor
			}
			
			ColorAnimation {
				duration: CallsStyle.entry.endCallAnimation.duration
				from: CallsStyle.entry.endCallAnimation.blinkColor
				to: CallsStyle.entry.color.normal
			}
		}
	}
}
