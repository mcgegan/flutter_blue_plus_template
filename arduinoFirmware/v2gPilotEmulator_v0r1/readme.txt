1. bluetooth characteristic의 property를 변경한 경우 Tools->board에서 
   보드를 esp32 계열 다른 보드로 변경해서 verify/compile 한 다음 
   다시 현재 사용 중인 board로 변경하고 다시 verify/compile해야 property 변경 내용 반영됨
   - 또는 C:\Users\user\AppData\Local\arduino\sketches temp folder 전체 또는 최신 folder 삭제
2. bluetooth service, characteristic 변경 사항이 반영 되지 않으면 연결한 장치 리부팅