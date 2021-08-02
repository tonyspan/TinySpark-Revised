template <typename T>
void HiddenLog(const T& arg)
{
	std::cout << arg << '\n';
}

template<typename T, typename... Args>
void HiddenLog(const T& arg, const Args&... rest)
{
	std::cout << arg << ' ';

	HiddenLog(rest...);
}

template<typename... Args>
void Log(const Args&... rest)
{
	HiddenLog(rest...);
}