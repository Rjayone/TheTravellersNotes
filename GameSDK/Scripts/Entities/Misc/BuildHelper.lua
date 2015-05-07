BuildHelper = 
{
	Properties = 
	{
		fileModel = "objects/default/primitive_pyramid.cgf",

	Towers = {
				obj_model_1 = "", 
				obj_model_2 = "", 
				obj_model_3 = "", 
				obj_model_4 = "", 
				sBuildingName_1 ="",			
				sBuildingName_2 ="",			
				sBuildingName_3 ="",			
				sBuildingName_4 ="",
			},
			
	Mines = {
				obj_model_1 = "", 
				obj_model_2 = "", 
				obj_model_3 = "", 
				obj_model_4 = "", 
				sBuildingName_1 ="",			
				sBuildingName_2 ="",			
				sBuildingName_3 ="",			
				sBuildingName_4 ="",
			},	

	Houses	 = {
				obj_model_1 = "", 
				obj_model_2 = "", 
				obj_model_3 = "", 
				obj_model_4 = "", 
				sBuildingName_1 ="",			
				sBuildingName_2 ="",			
				sBuildingName_3 ="",			
				sBuildingName_4 ="",
			},	
}, 

	Editor={
		Icon = "physicsobject.bmp",
		IconOnTop=1,
	  },
}

function BuildHelper:OnPropertyChange()
	self.bActive = self.Properties.bActive;
	BroadcastEvent(self, "OnPropertyChange");
end
