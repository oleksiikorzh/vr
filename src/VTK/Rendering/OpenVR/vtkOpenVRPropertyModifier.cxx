/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOpenVRPropertyModifier.cxx

Copyright (c) Ventura Romero Mendo
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOpenVRPropertyModifier.h"

#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkOpenVRRenderWindow.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkOpenVROverlay.h"
#include <valarray>
#include "vtkRenderWindowInteractor3D.h"

#include "vtkRenderer.h"
#include "vtkTextSource.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkSphereSource.h"
#include "vtkCylinderSource.h"
#include "vtkCubeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataAlgorithm.h"

#include "vtkOpenVRRenderWindowInteractor.h"
#include "vtkOpenVRRenderer.h"
#include "vtkInteractorStyle3D.h"
#include "vtkOpenVRInteractorStyleInputData.h"
#include "vtkOpenVRCamera.h"

vtkStandardNewMacro(vtkOpenVRPropertyModifier);



/*union GenericSource {
	vtkSphereSource *Sphere;
	vtkCylinderSource *Cylinder;
	vtkCubeSource *Cube;
	vtkPolyDataAlgorithm *Def;
	GenericSource() {};
	GenericSource(vtkSource s)
	{
		switch (s)
		{
		case vtkSource::Sphere: this->Sphere = vtkSphereSource::New(); break;
		case vtkSource::Cylinder: this->Cylinder = vtkCylinderSource::New(); break;
		case vtkSource::Cube: this->Cube = vtkCubeSource::New(); break;
		default: this->Def = vtkPolyDataAlgorithm::New(); break;
		}
	}
};*/
/*GenericSource(vtkSource s)
{
	switch (s)
	{
	case vtkSource::Sphere: this->Sphere = vtkSphereSource::New(); break;
	case vtkSource::Cylinder: this->Cylinder = vtkCylinderSource::New(); break;
	case vtkSource::Cube: this->Cube = vtkCubeSource::New(); break;
	default: this->Def = vtkPolyDataAlgorithm::New(); break;
	}
}*/




//----------------------------------------------------------------------------
vtkOpenVRPropertyModifier::vtkOpenVRPropertyModifier()
{
	//Dummy test
	this->TestSource = NULL;		// vtkPolyDataAlgorithm::New();		//this->TestSource = vtkSphereSource::New();
	this->SelectSourceDownCast(vtkSource::Sphere);
	//this->SelectSourceDownCast(vtkSource::Cube);

	this->TestActor = NULL;
	this->TestMapper = vtkPolyDataMapper::New();
	this->TestRenderer = NULL;

	
	if (this->TestMapper && this->TestSource)
	{
		this->TestMapper->SetInputConnection(this->TestSource->GetOutputPort());
	}
	



	//this->gs = GenericSource();

	


}

//----------------------------------------------------------------------------
vtkOpenVRPropertyModifier::~vtkOpenVRPropertyModifier()
{
	if (this->TestActor)
	{
		this->TestActor->Delete();
	}

	if (this->TestMapper)
	{
		this->TestMapper->Delete();
	}

	this->TestSource->Delete();
	this->TestSource = NULL;
}

//----------------------------------------------------------------------------
void vtkOpenVRPropertyModifier::ModifyProperty(vtkObject * obj, vtkField field, char* value)
{	//Add new parameter: vtkObjectType (which is a new enum class).
	//This will remove problems on downcasting:
	//instead of vtkSphereSource, we can select vtkAlgorithm or vtkPolyDataAlgorithm
	//�?To do so, create a method, selectDownCast(...) �?
	//BRIGHT IDEA: Use an "union" to store all the different types as pointers!!!
	vtkProp *downProp;
	vtkActor *downActor;
	vtkSphereSource *downSphere;
	vtkProp3D *downProp3D;

	switch(field)
	{
	case vtkField::Visibility:
//		downProp = vtkProp::SafeDownCast(obj);	//downSphere = vtkSphereSource::SafeDownCast(obj);
//		if (downProp /*downSphere*/ != NULL)
		if(obj->IsA("vtkProp"))			//WORKS WITH UPCAST!!
		{
			downProp = vtkProp::SafeDownCast(obj);

			vtkStdString str = vtkVariant(value).ToString();
			if (str.compare("true") == 0)
			{
				downProp->SetVisibility(true);
			}
			else if (str.compare("false") == 0)
			{
				downProp->SetVisibility(false);
			}
		}
		break;
	case vtkField::Opacity:
		downActor = vtkActor::SafeDownCast(obj);
		if (downActor != NULL)
		{
			downActor->GetProperty()->SetOpacity(vtkVariant(value).ToDouble());
		}
		break;
	case vtkField::Scale:
		downProp3D = vtkProp3D::SafeDownCast(obj);
		if(downProp3D != NULL)
		{
			downProp3D->SetScale(vtkVariant(value).ToDouble());
		}
		break;
	case vtkField::Radius:
//		downSphere = vtkSphereSource::SafeDownCast(obj);
//		if (downSphere != NULL)
		if (obj->IsA("vtkSphereSource"))			//will it WORK WITH downCAST!!
		{
			downSphere = vtkSphereSource::SafeDownCast(obj);

			downSphere->SetRadius(vtkVariant(value).ToDouble());
		}
		break;
	// Add cases as desired. Remenber to add the vtkProp in the enum class!
	default:
		vtkErrorMacro(<< "vtkField not recognized.");
	}
}

void vtkOpenVRPropertyModifier::InitTest()
{
	//create and place in coordinates.
//	TestSource->SetPhiResolution(20);
//	TestSource->SetThetaResolution(20);
	TestActor = vtkActor::New();
	TestActor->PickableOff();
	TestActor->DragableOff();
	TestActor->SetMapper(TestMapper);
	TestActor->GetProperty()->SetAmbient(1.0);
	TestActor->GetProperty()->SetDiffuse(0.0);
}

void vtkOpenVRPropertyModifier::ShowTest(vtkOpenVRRenderWindowInteractor *rwi)
{
	//Current renderer
	vtkOpenVRRenderer *ren = NULL;
	vtkInteractorStyle3D *ist = NULL;
	vtkOpenVRCamera *cam = NULL;
	int pointer;
	if (rwi)
	{
		int pointer = rwi->GetPointerIndex();		//pointer = rwi->GetPointerIndexLastTouchpad();
		ren = vtkOpenVRRenderer::SafeDownCast(rwi->FindPokedRenderer(rwi->GetEventPositions(pointer)[0],
																																 rwi->GetEventPositions(pointer)[1]));
		ist = vtkOpenVRInteractorStyleInputData::SafeDownCast(rwi->GetInteractorStyle());
		cam = vtkOpenVRCamera::SafeDownCast(ren->GetActiveCamera());
	}
	else return;
	
	//check if it is already active
	if (!TestActor)
	{
		this->InitTest();
	}

	//check if used different renderer to previous visualization
	if (ren != TestRenderer)
	{
		if (TestRenderer != NULL && TestActor)
		{
			TestRenderer->RemoveActor(TestActor);
		}
		if (ren)
		{
			ren->AddActor(TestActor);
		}
		else
		{
			vtkWarningMacro(<< "no current renderer on the interactor style.");
		}
		this->TestRenderer = ren;
	}
	
	if (rwi)
	{
		rwi->Render();
	}
}

void vtkOpenVRPropertyModifier::HideTest()
{
	if (TestRenderer != NULL && TestActor)
	{
		TestRenderer->RemoveActor(TestActor);
		this->TestRenderer = NULL;
	}
}

void vtkOpenVRPropertyModifier::SetGenericSource(vtkSource s)
{
	
}

void vtkOpenVRPropertyModifier::SelectSourceDownCast(vtkSource s)
{
	if (this->TestSource != NULL) this->TestSource->Delete();

	switch (s)
	{
	case vtkSource::Sphere: this->TestSource = vtkSphereSource::New(); break;
	case vtkSource::Cylinder: this->TestSource = vtkCylinderSource::New(); break;
	case vtkSource::Cube: this->TestSource = vtkCubeSource::New(); break;
	}
}

//----------------------------------------------------------------------------
void vtkOpenVRPropertyModifier::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);
}
	
/*
//----------------------------------------------------------------------------
void vtkInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Auto Adjust Camera Clipping Range "
		<< (this->AutoAdjustCameraClippingRange ? "On\n" : "Off\n");

	os << indent << "Pick Color: (" << this->PickColor[0] << ", "
		<< this->PickColor[1] << ", "
		<< this->PickColor[2] << ")\n";

	os << indent << "CurrentRenderer: " << this->CurrentRenderer << "\n";
	if (this->PickedRenderer)
	{
		os << indent << "Picked Renderer: " << this->PickedRenderer << "\n";
	}
	else
	{
		os << indent << "Picked Renderer: (none)\n";
	}
	if (this->CurrentProp)
	{
		os << indent << "Current Prop: " << this->CurrentProp << "\n";
	}
	else
	{
		os << indent << "Current Actor: (none)\n";
	}

	os << indent << "Interactor: " << this->Interactor << "\n";
	os << indent << "Prop Picked: " <<
		(this->PropPicked ? "Yes\n" : "No\n");

	os << indent << "State: " << this->State << endl;
	os << indent << "UseTimers: " << this->UseTimers << endl;
	os << indent << "HandleObservers: " << this->HandleObservers << endl;
	os << indent << "MouseWheelMotionFactor: " << this->MouseWheelMotionFactor << endl;

	os << indent << "Timer Duration: " << this->TimerDuration << endl;

	os << indent << "TDxStyle: ";
	if (this->TDxStyle == 0)
	{
		os << "(none)" << endl;
	}
	else
	{
		this->TDxStyle->PrintSelf(os, indent.GetNextIndent());
	}
}



//----------------------------------------------------------------------------
void vtkRenderWindowInteractor::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);

	os << indent << "InteractorStyle:    " << this->InteractorStyle << "\n";
	os << indent << "RenderWindow:    " << this->RenderWindow << "\n";
	if (this->Picker)
	{
		os << indent << "Picker: " << this->Picker << "\n";
	}
	else
	{
		os << indent << "Picker: (none)\n";
	}
	if (this->ObserverMediator)
	{
		os << indent << "Observer Mediator: " << this->ObserverMediator << "\n";
	}
	else
	{
		os << indent << "Observer Mediator: (none)\n";
	}
	os << indent << "LightFollowCamera: " << (this->LightFollowCamera ? "On\n" : "Off\n");
	os << indent << "DesiredUpdateRate: " << this->DesiredUpdateRate << "\n";
	os << indent << "StillUpdateRate: " << this->StillUpdateRate << "\n";
	os << indent << "Initialized: " << this->Initialized << "\n";
	os << indent << "Enabled: " << this->Enabled << "\n";
	os << indent << "EnableRender: " << this->EnableRender << "\n";
	os << indent << "EventPosition: " << "( " << this->EventPosition[0] <<
		", " << this->EventPosition[1] << " )\n";
	os << indent << "LastEventPosition: " << "( " << this->LastEventPosition[0]
		<< ", " << this->LastEventPosition[1] << " )\n";
	os << indent << "EventSize: " << "( " << this->EventSize[0] <<
		", " << this->EventSize[1] << " )\n";
	os << indent << "Viewport Size: " << "( " << this->Size[0] <<
		", " << this->Size[1] << " )\n";
	os << indent << "Number of Fly Frames: " << this->NumberOfFlyFrames << "\n";
	os << indent << "Dolly: " << this->Dolly << "\n";
	os << indent << "ControlKey: " << this->ControlKey << "\n";
	os << indent << "AltKey: " << this->AltKey << "\n";
	os << indent << "ShiftKey: " << this->ShiftKey << "\n";
	os << indent << "KeyCode: " << this->KeyCode << "\n";
	os << indent << "KeySym: " << (this->KeySym ? this->KeySym : "(null)")
		<< "\n";
	os << indent << "RepeatCount: " << this->RepeatCount << "\n";
	os << indent << "Timer Duration: " << this->TimerDuration << "\n";
	os << indent << "TimerEventId: " << this->TimerEventId << "\n";
	os << indent << "TimerEventType: " << this->TimerEventType << "\n";
	os << indent << "TimerEventDuration: " << this->TimerEventDuration << "\n";
	os << indent << "TimerEventPlatformId: " << this->TimerEventPlatformId << "\n";
	os << indent << "UseTDx: " << this->UseTDx << endl;
	os << indent << "Recognize Gestures: " << this->RecognizeGestures << endl;
}
*/