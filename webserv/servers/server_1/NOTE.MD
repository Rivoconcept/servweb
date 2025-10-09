
init-dirs:
	@if [ ! -d "$(DATA_PATH)/db_data" ]; then \
		mkdir -p "$(DATA_PATH)/db_data"; \
		echo "Create: $(DATA_PATH)/db_data"; \
	fi
	@if [ ! -d "$(DATA_PATH)/wp_data" ]; then \
		mkdir -p "$(DATA_PATH)/wp_data"; \
		echo "Create: $(DATA_PATH)/wp_data"; \
	fi
	@if [ "$$(stat -c '%U' $(DATA_PATH))" != "$(USER)" ]; then \
		echo "Fixing ownership of $(DATA_PATH)"; \
		sudo chown -R $(USER):$(USER) $(DATA_PATH); \
	fi
	@chmod 755 "$(DATA_PATH)"