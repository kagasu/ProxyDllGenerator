#pragma once
class FunctionInfo
{
public:
	uint32_t index;
	std::string guid;
	std::string name;

	FunctionInfo(uint32_t index, std::string guid, std::string name)
	{
		this->index = index;
		this->guid = guid;
		this->name = name;
	}
};
