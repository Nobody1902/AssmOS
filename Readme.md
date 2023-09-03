# AssmOS

- A very simple 64 bit operating system

# Build

- First run `docker build buildenv -t assm-os.buildenv`

  ##### Windows

  - Run `docker run --rm -it -v %cd%:/root/env assm-os.buildenv`

  ##### Linux & MacOS

  - Run `docker run --rm -it -v $(pwd):/root/env assm-os.buildenv`

- Then run `make build` and exit from the docker container (run `exit`)

- You'll find the `kernel.iso` file in `dist/x86_64`

##### Qemu

- Run `qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso`

# Credits

- Blog [os.phil-oop.com](https://os.phil-opp.com/edition-1/)
- Youtube [@CodePulse](https://www.youtube.com/@CodePulse)
