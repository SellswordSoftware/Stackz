# PlaydateCStarterDevcontainerApline
A hello world starter project for Lua Playdate games that uses an light debian devcontainer for the dev environment to stay sleek and small to run performantly anywhere without the need to install dev tools. Getting alpine to build with simulater compatible glibc shared libraries was not worth it compared to the speed of setup of just using a prebuilt deb image.

This assumes you are on some fairly normal distro of linux, you already have the sdk installed and have it located at /etc/PlaydateSDK and docker installed. The point of this starter is it relies on vs code devcontainers.

This is pretty much just for me so I have a repeatable containerized dev environment that will get me off the ground quickly for new projects regardless of what pc I'm on and not having to worry about build tools.

I'm sure if I was smart enough I could get the simulator to run inside the container too and have it route to the host but for now you need the simulator installed locally and for it to be added to you path.

In order to launch the simulator from the container, it uses local ssh and uses a shared volume of your ~/.ssh into the container so as long as you have localhost ssh set up, the container should be able to do so as well. If you have localhost ssh set up then ./run.sh should just "work" and luanch the pdx on your local system's simulator outside the devcontainer. Otherwise, ./build.sh should craft your pdx that you can then do with what you will.