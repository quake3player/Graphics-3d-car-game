BUILD_DIR := build

.PHONY: all configure build game run clean

all: build

configure:
	cmake -S . -B $(BUILD_DIR)

build: configure
	cmake --build $(BUILD_DIR) --target scene_game

game: build

run: build
	./$(BUILD_DIR)/SceneGame/scene_game $(if $(EXTRAS),--extras) $(if $(SHADOWS),--shadows)

clean:
	rm -rf $(BUILD_DIR)
