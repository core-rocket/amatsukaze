#ifndef ES920LR_HPP_
#define ES920LR_HPP_

#include <Arduino.h>
#include <stdint.h>

template<typename Serial, size_t buffer_length=50>
class ES920LR {
public:
	ES920LR(Serial *serial) : serial(serial), rssi_on(false) {}

	enum class Response {
		OK,
		NG,
		VER,	// binary formatだとOKあるいはNG -> これらはどうなるねん
		JOIN,
		WAKE,
	};

	auto recv() -> void;
	auto send() -> bool;
	auto get_response() -> Response;	// formatは普通の受信と同じ

	auto recv_text() -> void {
		if(rssi_on) read_rssi();
		// TODO CRLFが来るまでデータを読む
	}

	auto recv_binary() -> void {
		// TODO 出力長を読む
		if(rssi_on) read_rssi();
		// TODO 出力長-4だけデータを読む
	}

	auto send_text(const char *str) -> bool {
		// ASCIIでデータを入力
		// 終端はCRLF
		return false;
	}

	auto send_binary() -> bool {
		// TODO 1byte入力長を入力
		// TODO 入力長だけデータを入力(最大50byte)
		return false;
	}

	auto read_rssi() -> void {
		// TODO 4byteのRSSI値を読む(ASCII)
	}
private:
	Serial *serial;
	bool rssi_on;
	uint8_t rssi[4];
	uint8_t buf[buffer_length];
};

#endif
