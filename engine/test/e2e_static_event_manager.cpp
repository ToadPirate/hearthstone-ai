#include <iostream>
#include "Utils/StaticEventTriggerer.h"

namespace impl
{
	namespace GameStart
	{
		class Handler1
		{
		public:
			static void Trigger()
			{
				std::cout << "GameStart::Handler1 called" << std::endl;
			}
		};

		class Handler2
		{
		public:
			static void Trigger()
			{
				std::cout << "GameStart::Handler2 called" << std::endl;
			}
		};
	}
}

int main(void)
{
	Utils::StaticEventTriggerer<
		impl::GameStart::Handler1,
		impl::GameStart::Handler2>::Trigger();

	return 0;
}