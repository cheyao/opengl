#pragma once

class System {
      public:
	System();
	System(System&&) = delete;
	System(const System&) = delete;
	System& operator=(System&&) = delete;
	System& operator=(const System&) = delete;
	virtual ~System();

	virtual void update(const float delta) = 0;

      private:
};
