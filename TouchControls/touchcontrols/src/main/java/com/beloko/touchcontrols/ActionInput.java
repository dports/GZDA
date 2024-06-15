package com.beloko.touchcontrols;

import com.beloko.touchcontrols.ControlConfig.Type;

import java.io.Serializable;


public class ActionInput implements Serializable
{ 
	private static final long serialVersionUID = 1L;

	public String tag; 
	public String description;
	public boolean invert;
	public float scale = 1; //senstivty for analog

	public int source = -1;
	public Type sourceType;
	public boolean sourcePositive=true; //Used when using analog as a button
	
	public int actionCode;
	public Type actionType;

	public ActionInput(String t,String n,int action,Type actiontype)
	{
		tag = t;
		description = n;
		actionCode = action;
		actionType = actiontype;
	}

	public ActionInput(String t,String n,int action,Type actiontype,Type sourceTypeDef,int sourceDef)
	{
		tag = t;
		description = n;
		actionCode = action;
		actionType = actiontype;

		sourceType = sourceTypeDef;
		source = sourceDef;
	}

	public ActionInput(String t,String n,int action,Type actiontype,Type sourceTypeDef,int sourceDef,boolean sourcePosDef)
	{
		tag = t;
		description = n;
		actionCode = action;
		actionType = actiontype;

		sourceType = sourceTypeDef;
		source = sourceDef;
		sourcePositive = sourcePosDef;
	}

	public String toString()
	{
		return description + ":" + sourceType.toString() + " source: " + source + " sourcePositive: " + sourcePositive;
	}
}

