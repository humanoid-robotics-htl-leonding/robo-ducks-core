import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaoComponent } from './nao.component';

describe('NaoComponent', () => {
  let component: NaoComponent;
  let fixture: ComponentFixture<NaoComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaoComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaoComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
